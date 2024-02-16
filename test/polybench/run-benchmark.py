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


def TryParseStartLine(state: dict, line: str) -> bool:
	PTYPE_REGEX     = r'\[(\w+)\]\s*Starting to run Decent WASM program\s*\(type=(\w+)\)\.\.\.'

	m = re.search(PTYPE_REGEX, line)
	if m is None:
		return False
	else:
		# found the beginning of a test run
		# ensure states are clean
		assert state['currEnv'] == '', f'Env not clean: {state["currEnv"]}'
		assert state['currPType'] == '', f'PType not clean: {state["currPType"]}'
		assert len(state['measurements']) == 0, f'Measurements not clean: {state["measurements"]}'

		# record the labels of the new test run
		state['currEnv'] = m.group(1)
		state['currPType'] = m.group(2)
		return True


def TryParseTimeLine(state: dict, line: str) -> bool:
	TIME_REGEX      = r'\[(\w+)\]\s*Benchmark stopped\.\s*\(\s*Started\s+\@\s+(\d+)\s+us\s*,\s*ended\s+\@\s+(\d+)\s+us\s*,\s+spent\s+(\d+)\s+us\s*\)'
	m = re.search(TIME_REGEX, line)
	if m is None:
		return False
	else:
		# found the line with time printout
		if len(state['measurements']) > 0:
			raise ValueError('Found multiple time printouts in a single test run')
		assert state['currEnv'] == m.group(1), f'Env mismatch: {state["currEnv"]} != {m.group(1)}'
		state['measurements'] = [
			int(m.group(2)),
			int(m.group(3)),
			int(m.group(4)),
		]
		paraPart = line[line.find('('):]
		print(f'{paraPart} \t {state["measurements"]}') # print to double check
		return True


def TryParseCounterLine(state: dict, line: str) -> bool:
	THRESHOLD_REGEX = r'\[(\w+)\]\s*Threshold\s*:\s*(\d+)\s*,\s*Counter\s*:\s*(\d+)\s*'

	m = re.search(THRESHOLD_REGEX, line)
	if m is None:
		return False
	else:
		# found the line with counter printout
		if len(state['measurements']) != 3:
			raise ValueError('Found counter printout without time printout')
		if state['currPType'] != 'instrumented':
			raise ValueError('Found counter printout in a non-instrumented test run')
		assert state['currEnv'] == m.group(1), f'Env mismatch: {state["currEnv"]} != {m.group(1)}'
		state['measurements'].append([
			int(m.group(2)),
			int(m.group(3)),
		])
		return True


def TryParseEndLine(state: dict, line: str) -> bool:
	PTYPE_REGEX     = r'\[(\w+)\]\s*Finished to run Decent WASM program\s*\(type=(\w+)\)\.\.\.'

	m = re.search(PTYPE_REGEX, line)
	if m is None:
		return False
	else:
		# found the beginning of a test run
		# ensure labels are consistent
		assert state['currEnv'] == m.group(1), f'Env mismatch: {state["currEnv"]} != {m.group(1)}'
		assert state['currPType'] == m.group(2), f'PType mismatch: {state["currPType"]} != {m.group(2)}'

		# recording the test run results
		if len(state['measurements']) == 0:
			raise ValueError('Found a test run without time printout')
		if state['currPType'] == 'instrumented' and len(state['measurements']) != 4:
			raise ValueError('Found an instrumented test run without counter printout')
		state['res'][state['currEnv']][state['currPType']].append(state['measurements'])

		# reset state
		state['currEnv'] = ''
		state['currPType'] = ''
		state['measurements'] = []

		return True


def ParseAllEnvTimePrintout(
	printoutLines: List[str]
) -> Dict[str, List[int]]:


	state = {
		'res': {
			'Untrusted': {
				'plain': [],
				'instrumented': [],
			},
			'Enclave': {
				'plain': [],
				'instrumented': [],
			},
			'Native': {
				'plain': [],
			},
		},

		'currEnv'  : '',
		'currPType': '',

		'measurements': [],
	}

	for line in printoutLines:
		if TryParseStartLine(state, line):
			continue
		elif TryParseTimeLine(state, line):
			continue
		elif TryParseCounterLine(state, line):
			continue
		elif TryParseEndLine(state, line):
			continue

	return state['res']


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
	REPEAT_TIMES = 1

	output = {
		'measurement': {},
		'raw': {},
	}

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

