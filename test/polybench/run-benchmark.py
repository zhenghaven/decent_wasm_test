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

from typing import Dict, List, Tuple


CURR_DIR = os.path.dirname(os.path.abspath(__file__))
PROJ_BUILD_DIR = os.path.join(CURR_DIR, os.pardir, os.pardir, 'build')
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
	printoutLines: List[str]
) -> Tuple[int, int, int]:
	benchStart = f'[{env}] Benchmark started'
	benchStop = f'[{env}] Benchmark stopped'

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

	benchStopLine = printoutLines[i + 1]

	TIME_REGEX = r'\(\s*Started\s+\@\s+(\d+)\s+us\s*,\s*ended\s+\@\s+(\d+)\s+us\s*,\s+spent\s+(\d+)\s+us\s*\)'
	m = re.search(TIME_REGEX, benchStopLine)
	if m is None:
		raise ValueError('Cannot parse benchmark stop line')

	res =  [
		int(m.group(1)),
		int(m.group(2)),
		int(m.group(3)),
	]

	print(benchStopLine, res)

	return res


def SplitLines(
	inLines: List[str],
	splitLine: str
) -> Tuple[List[str], List[str]]:

	for i in range(len(inLines)):
		if splitLine in inLines[i]:
			return inLines[:i], inLines[i:]

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
		'plain': ParseTimePrintout(env, plainPrintoutLines),
		# 'inst':  [ inst_begin, inst_end, inst_duration ],
		'inst':  ParseTimePrintout(env, instPrintoutLines),
	}


def ParseAllEnvTimePrintout(
	printoutLines: List[str]
) -> Dict[str, List[int]]:

	enclaveLaunch = '[Enclave] Running plain wasm'

	untrustedPrintoutLines, enclavePrintoutLines = SplitLines(
		printoutLines,
		enclaveLaunch
	)

	return {
		# 'untrusted': {
		#    'plain': [ plain_begin, plain_end, plain_duration ],
		#    'inst':  [ inst_begin, inst_end, inst_duration ],
		# }
		'untrusted': ParseOneEnvTimePrintout('Untrusted', untrustedPrintoutLines),
		# 'enclave': {
		#    'plain': [ plain_begin, plain_end, plain_duration ],
		#    'inst':  [ inst_begin, inst_end, inst_duration ],
		# }
		'enclave': ParseOneEnvTimePrintout('Enclave', enclavePrintoutLines),
	}


REPEAT_TIMES = 3


for testCase in TEST_CASES:
	testCasePath = os.path.join(CURR_DIR, testCase)
	benchmarkPath = os.path.join(BENCHMARK_BUILD_DIR, BENCHMARKER_BIN)

	output['raw'][testCase] = []
	output['measurement'][testCase] = []

	cmd = [
		benchmarkPath,
		testCasePath + '.wasm',
		testCasePath + '.nopt.wasm',
	]
	print(' '.join(cmd))

	for i in range(REPEAT_TIMES):
		with subprocess.Popen(
			cmd,
			stdout=subprocess.PIPE,
			stderr=subprocess.PIPE,
			cwd=BENCHMARK_BUILD_DIR,
		) as proc:
			stdout, stderr = proc.communicate()
			stdout = stdout.decode('utf-8', errors='replace')
			stderr = stderr.decode('utf-8', errors='replace')

			output['raw'][testCase].append({
				'stdout': stdout,
				'stderr': stderr,
				'returncode': proc.returncode,
			})

			if proc.returncode != 0:
				print('Benchmark failed for: ' + testCase)

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

				exit(1)

			printoutLines = stdout.splitlines()
			output['measurement'][testCase].append(ParseAllEnvTimePrintout(printoutLines))


	with open(os.path.join(PROJ_BUILD_DIR, 'benchmark.json'), 'w') as f:
		json.dump(output, f, indent='\t')

