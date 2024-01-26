#!/usr/bin/env python3
# -*- coding:utf-8 -*-
###
# Copyright (c) 2024 Haofan Zheng
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.
###


import os
import subprocess


CURR_DIR = os.path.dirname(os.path.abspath(__file__))
BENCHMARK_BUILD_DIR = os.path.join(CURR_DIR, os.pardir, os.pardir, 'build', 'src')
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


for testCase in TEST_CASES:
	testCasePath = os.path.join(CURR_DIR, testCase)
	benchmarkPath = os.path.join(BENCHMARK_BUILD_DIR, BENCHMARKER_BIN)

	cmd = [
		benchmarkPath,
		testCasePath + '.wasm',
		testCasePath + '.nopt.wasm',
	]
	print(' '.join(cmd))

	with subprocess.Popen(
		cmd,
		stdout=subprocess.PIPE,
		stderr=subprocess.PIPE,
		cwd=BENCHMARK_BUILD_DIR,
	) as proc:
		stdout, stderr = proc.communicate()
		stdout = stdout.decode('utf-8', errors='replace')
		if proc.returncode != 0:
			print('Benchmark failed for: ' + testCase)
			stderr = stderr.decode('utf-8', errors='replace')

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

	stdOutLines = stdout.splitlines()
	timePrintout = [
		x for x in stdOutLines if (
			('Benchmark stopped. ' in x) or
			('Running plain wasm' in x) or
			('Running instrumented wasm' in x)
		)
	]
	assert len(timePrintout) == 8, 'There should be 4 benchmark time printouts'

	print('\n'.join(timePrintout))

