#!/usr/bin/env python3
# -*- coding:utf-8 -*-
###
# Copyright (c) 2024 Haofan Zheng
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.
###


import json
import os
import re
import subprocess
import sys
import time

from typing import Dict, List, Tuple

NICE_ADJUST = -20
AFFINITY = { 3,}

CURR_DIR = os.path.dirname(os.path.abspath(__file__))
PROJ_BUILD_DIR = os.path.join(CURR_DIR, os.pardir, os.pardir, 'build-release')
BENCHMARK_BUILD_DIR = os.path.join(PROJ_BUILD_DIR, 'src')
BENCHMARKER_BIN = 'decent_wasm_test'
TEST_CASES = [
	# datamining - 2
	'correlation',
	'covariance',

	# linear-algebra/blas - 7
	'gemm',
	'gemver',
	'gesummv',
	'symm',
	'syr2k',
	'syrk',
	'trmm',

	# linear-algebra/kernels - 6
	'2mm',
	'3mm',
	'atax',
	'bicg',
	'doitgen',
	'mvt',

	# linear-algebra - solvers - 6
	'cholesky',
	'durbin',
	'gramschmidt',
	'lu',
	'ludcmp',
	'trisolv',

	# medley - 3
	'deriche',
	'floyd-warshall',
	'nussinov',

	# stencils - 6
	'adi',
	'fdtd-2d',
	'heat-3d',
	'jacobi-1d',
	'jacobi-2d',
	'seidel-2d',
]


output = {
	'measurement': {},
	'raw': {},
}


def ParseTimePrintout(
	env: str,
	printoutLines: List[str],
	parseCounter: bool = False
) -> list:
	benchStart = f'[{env}] Benchmark started'
	benchStop = f'[{env}] Benchmark stopped'
	threshold = f'[{env}] Threshold:'

	i = 0
	for line in printoutLines:
		if benchStart in line:
			break
		i += 1

	if i == len(printoutLines):
		raise ValueError('Cannot find benchmark start line')

	if (
		i + 1 == len(printoutLines) or
		benchStop not in printoutLines[i + 1]
	):
		raise ValueError('Cannot find benchmark stop line')

	i += 1
	benchStopLine = printoutLines[i]

	TIME_REGEX = r'\(\s*Started\s+\@\s+(\d+)\s+us\s*,\s*ended\s+\@\s+(\d+)\s+us\s*,\s+spent\s+(\d+)\s+us\s*\)'
	m = re.search(TIME_REGEX, benchStopLine)
	if m is None:
		raise ValueError('Cannot parse benchmark stop line')

	res =  [
		int(m.group(1)),
		int(m.group(2)),
		int(m.group(3)),
	]

	if parseCounter:
		i += 1
		for line in printoutLines[i:]:
			if threshold in line:
				break
			i += 1

		if i == len(printoutLines):
			raise ValueError('Cannot find threshold line')

		thresholdLine = printoutLines[i]
		THRESHOLD_REGEX = r'\s+Threshold\s*:\s*(\d+)\s*,\s*Counter\s*:\s*(\d+)\s*'
		m = re.search(THRESHOLD_REGEX, thresholdLine)
		if m is None:
			raise ValueError(f'Cannot parse threshold line "{thresholdLine}"')

		res.append([
			int(m.group(1)),
			int(m.group(2)),
		])

	print(benchStopLine, res)

	return res


def SplitLines(
	inLines: List[str],
	splitLine: str
) -> Tuple[List[str], List[str]]:

	for i in range(len(inLines)):
		if splitLine in inLines[i]:
			return inLines[:i], inLines[i:]

	print('\n'.join(inLines))
	raise ValueError(f'Cannot find split line: {splitLine}')


def ParseOneEnvTimePrintout(
	env: str,
	printoutLines: List[str]
) -> Dict[str, List[int]]:

	# plainLaunch = f'[{env}] Running plain wasm'
	instLaunch = f'[{env}] Running instrumented wasm'

	plainPrintoutLines, instPrintoutLines = SplitLines(
		printoutLines,
		instLaunch
	)

	return {
		# 'plain': [ plain_begin, plain_end, plain_duration ],
		'plain': ParseTimePrintout(env, plainPrintoutLines, parseCounter=False),
		# 'inst':  [ inst_begin, inst_end, inst_duration ],
		'inst':  ParseTimePrintout(env, instPrintoutLines, parseCounter=True),
	}


def ParseAllEnvTimePrintout(
	printoutLines: List[str]
) -> Dict[str, List[int]]:

	enclaveLaunch = '[Enclave] Running plain wasm'
	nativeLaunch = '[Native] [PolyBench]'

	untrustedPrintoutLines, enclavePrintoutLines = SplitLines(
		printoutLines,
		enclaveLaunch
	)
	enclavePrintoutLines, nativePrintoutLines = SplitLines(
		enclavePrintoutLines,
		nativeLaunch
	)

	return {
		# 'Untrusted': {
		#    'plain': [ plain_begin, plain_end, plain_duration ],
		#    'inst':  [ inst_begin, inst_end, inst_duration ],
		# }
		'Untrusted': ParseOneEnvTimePrintout('Untrusted', untrustedPrintoutLines),
		# 'Enclave': {
		#    'plain': [ plain_begin, plain_end, plain_duration ],
		#    'inst':  [ inst_begin, inst_end, inst_duration ],
		# }
		'Enclave': ParseOneEnvTimePrintout('Enclave', enclavePrintoutLines),
		# 'Native': {
		#    'plain': [ plain_begin, plain_end, plain_duration ],
		# }
		'Native': {
			'plain': ParseTimePrintout('Native', nativePrintoutLines, parseCounter=False)
		},
	}


def SetPriorityAndAffinity() -> None:
	# Set nice
	os.nice(NICE_ADJUST)

	# Set real-time priority
	schedParam = os.sched_param(os.sched_get_priority_max(os.SCHED_FIFO))
	os.sched_setscheduler(0, os.SCHED_FIFO, schedParam)

	# Set affinity
	os.sched_setaffinity(0, AFFINITY)


def RunProgram(cmd: List[str]) -> Tuple[str, str, int]:
	cmdStr = ' '.join(cmd)
	print(f'Running: {cmdStr}')

	with subprocess.Popen(
		cmd,
		stdout=subprocess.PIPE,
		stderr=subprocess.PIPE,
		cwd=BENCHMARK_BUILD_DIR,
		preexec_fn=lambda : SetPriorityAndAffinity(),
	) as proc:
		stdout, stderr = proc.communicate()
		stdout = stdout.decode('utf-8', errors='replace')
		stderr = stderr.decode('utf-8', errors='replace')

		time.sleep(5) # Wait for the system to settle down

		if proc.returncode != 0:
			print('Benchmark failed')

			print('##################################################')
			print('## STDOUT')
			print('##################################################')
			print(stdout)
			print()

			print('##################################################')
			print('## STDERR')
			print('##################################################')
			print(stderr)
			print()

			raise RuntimeError('Benchmark failed')

		return stdout, stderr, proc.returncode


def RunTestsAndCollectData() -> None:
	REPEAT_TIMES = 3

	for testCase in TEST_CASES:
		testCasePath = os.path.join(CURR_DIR, testCase)
		benchmarkPath = os.path.join(BENCHMARK_BUILD_DIR, BENCHMARKER_BIN)
		nativePath = os.path.join(CURR_DIR, testCase + '.app')

		output['raw'][testCase] = []
		output['measurement'][testCase] = []

		decentCmd = [
			benchmarkPath,
			testCasePath + '.wasm',
			testCasePath + '.nopt.wasm',
		]

		nativeCmd = [ nativePath ]

		for i in range(REPEAT_TIMES):
				decentStdout, decentStderr, decentRetcode = RunProgram(decentCmd)
				nativeStdout, nativeStderr, nativeRetcode = RunProgram(nativeCmd)

				stdout = decentStdout + '\n' + nativeStdout
				stderr = decentStderr + '\n' + nativeStderr
				assert decentRetcode == nativeRetcode

				output['raw'][testCase].append({
					'stdout': stdout,
					'stderr': stderr,
					'returncode': decentRetcode,
				})
				printoutLines = stdout.splitlines()
				output['measurement'][testCase].append(ParseAllEnvTimePrintout(printoutLines))


		with open(os.path.join(PROJ_BUILD_DIR, 'benchmark.json'), 'w') as f:
			json.dump(output, f, indent='\t')


def ReProcRawData(jsonFilePath: str) -> None:
	with open(jsonFilePath, 'r') as f:
		jsonFile = json.load(f)

	rawData = jsonFile['raw']

	newMeasurements = {}

	for testCase, results in rawData.items():
		newMeasurements[testCase] = []
		for repeatRes in results:
			printoutLines = repeatRes['stdout'].splitlines()
			newMeasurements[testCase].append(ParseAllEnvTimePrintout(printoutLines))

	jsonFile['measurement'] = newMeasurements

	with open(jsonFilePath, 'w') as f:
		json.dump(jsonFile, f, indent='\t')


def main() -> None:
	if len(sys.argv) > 1:
		if sys.argv[1] == 'reproc':
			ReProcRawData(sys.argv[2])
			return
		else:
			print('Unknown command')
			return
	else:
		RunTestsAndCollectData()


if __name__ == '__main__':
	main()

