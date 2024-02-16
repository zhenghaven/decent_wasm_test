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
import statistics
from typing import Any, List, Tuple

import plotly.graph_objects as go


CURR_DIR = os.path.dirname(os.path.abspath(__file__))
PROJ_BUILD_DIR = os.path.join(CURR_DIR, os.pardir, os.pardir, 'build-release')
BENCH_FILE_BASE = os.path.join(PROJ_BUILD_DIR, 'benchmark-202402160033')
BENCH_RES_FILE = BENCH_FILE_BASE + '.json'
GRAPH_FILE_BASE = BENCH_FILE_BASE

WARMUP_TIMES = 2


with open(BENCH_RES_FILE, 'r') as f:
	benchmarkRes = json.load(f)

measurements = benchmarkRes['measurement']

testCases = [ x for x in measurements.keys() ]


def SelectDataset(
	env: str,
	group: str,
	measurements: dict
) -> dict:
	# select all the elapsed time for each test case
	dataset = {
		k: [ x[2] for x in v[0][env][group] ]
			for k,v in measurements.items()
	}
	# remove the warmup times
	dataset = {
		k: v[WARMUP_TIMES:]
			for k,v in dataset.items()
	}
	# sort the valid data
	dataset = {
		k: sorted(v)
			for k,v in dataset.items()
	}

	# print the result in text
	print(f'{env} - {group} dataset:')
	for k, vs in dataset.items():
		errPlus = vs[-1] - statistics.median(vs)
		errPlus = errPlus / 1000
		errMinus = statistics.median(vs) - vs[0]
		errMinus = errMinus / 1000

		outStr = f'{k:20}: Elapsed '
		for v in vs:
			v = v / 1000
			outStr += f'{v:10.3f}ms, '
		outStr += 'Errors: -' + f'{errMinus:10.3f}ms, +' + f'{errPlus:10.3f}ms'
		print(outStr)

	return dataset


def DatasetNormalize(dataset: dict, baseDataset: dict) -> dict:
	assert len(dataset) == len(baseDataset), 'Dataset length mismatch'
	assert set(dataset.keys()) == set(baseDataset.keys()), 'Dataset key mismatch'

	res = {}
	for k, vs in dataset.items():
		baseVs = baseDataset[k]
		baseVMed = statistics.median(baseVs)
		#baseVAvg = statistics.mean(baseVs)
		res[k] = [ v / baseVMed for v in vs ]

	return res


def DatasetPlotBar(
	name: str,
	xData: List[Any],
	yDataset: dict
) -> go.Bar:
	yMid = [ statistics.median(yDataset[x]) for x in xData ] # if x != 'floyd-warshall'
	yPlus = [ (yDataset[x][-1] - statistics.median(yDataset[x])) for x in xData ] # if x != 'floyd-warshall'
	yMinus = [ (statistics.median(yDataset[x]) - yDataset[x][0]) for x in xData ] # if x != 'floyd-warshall'
	return go.Bar(
		name=name,
		x=xData,
		y=yMid,
		error_y=dict(
			type='data',
			symmetric=False,
			array=yPlus,
			arrayminus=yMinus,
		),
	)

# Collecting data
## Native - Plain
nativePlainDurations = (SelectDataset(
	'Native',
	'plain',
	benchmarkRes['measurement']
))
## Untrusted - Plain
untrustedPlainDurations = (SelectDataset(
	'Untrusted',
	'plain',
	benchmarkRes['measurement']
))
# Untrusted - Instrumented
untrustedInstDurations = (SelectDataset(
	'Untrusted',
	'instrumented',
	benchmarkRes['measurement']
))
# Enclave - Plain
enclavePlainDurations = (SelectDataset(
	'Enclave',
	'plain',
	benchmarkRes['measurement']
))
# Enclave - Instrumented
enclaveInstDurations = (SelectDataset(
	'Enclave',
	'instrumented',
	benchmarkRes['measurement']
))


# Normalizing data
untrustedPlainDurations = DatasetNormalize(
	untrustedPlainDurations,
	nativePlainDurations,
)
untrustedInstDurations = DatasetNormalize(
	untrustedInstDurations,
	nativePlainDurations,
)
enclavePlainDurations = DatasetNormalize(
	enclavePlainDurations,
	nativePlainDurations,
)
enclaveInstDurations = DatasetNormalize(
	enclaveInstDurations,
	nativePlainDurations,
)


# Plotting Bars
untrustedPlainBar = DatasetPlotBar(
	'Untrusted Plain',
	testCases,
	untrustedPlainDurations,
)
untrustedInstBar = DatasetPlotBar(
	'Untrusted Instrumented',
	testCases,
	untrustedInstDurations,
)
enclavePlainBar = DatasetPlotBar(
	'Enclave Plain',
	testCases,
	enclavePlainDurations,
)
enclaveInstBar = DatasetPlotBar(
	'Enclave Instrumented',
	testCases,
	enclaveInstDurations,
)


fig = go.Figure(
	data=[
		untrustedPlainBar,
		untrustedInstBar,
		enclavePlainBar,
		enclaveInstBar,
	],
)
fig.update_layout(barmode='group')

# Add horizontal line to represent the native runtime
fig.add_hline(
	y=1.0,
)
fig.add_annotation(dict(
	font=dict(color="black",size=12),
	#x=x_loc,
	#x=LabelDateB,
	x=1.04,
	y=1.0,
	showarrow=False,
	text='<b>Native<br>Plain</b>',
	textangle=0,
	xref="paper",
	yref="y",
))

fig.update_layout(
	autosize=False,
	width=1200,
	height=600,
)

fig.update_layout(legend=dict(
	orientation = "h",
	xanchor = "center",
	x = 0.5,
	y = -0.25,
))

fig.update_layout(
	margin=dict(l=40, r=80, t=40, b=40),
)

# fig.update_yaxes(type="log")

fig.write_image(GRAPH_FILE_BASE + '.png')

