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
PROJ_BUILD_DIR = os.path.join(CURR_DIR, os.pardir, os.pardir, 'build')
BENCH_RES_FILE = os.path.join(PROJ_BUILD_DIR, 'benchmark.json')
GRAPH_FILE_BASE = os.path.join(PROJ_BUILD_DIR, 'benchmark')

REPEAT_TIMES = 3


with open(BENCH_RES_FILE, 'r') as f:
	benchmarkRes = json.load(f)

measurements = benchmarkRes['measurement']

testCases = [ x for x in measurements.keys() ]


def SelectDataset(
	env: str,
	group: str,
	measurements: dict
) -> dict:
	dataset = {
			k: [
			v[i][env][group][2] for i in range(REPEAT_TIMES)
		] for k,v in measurements.items()
	}
	dataset = {
		k: sorted(v) for k,v in dataset.items()
	}
	return dataset


def DatasetUs2Ms(dataset: dict) -> dict:
	return {
		k: [ x / 1000 for x in v ] for k,v in dataset.items()
	}


def DatasetRange(*args: dict) -> Tuple[Any, Any, Any]:
	allValues = [ x for arg in args for x in arg.values() ]
	allValues = sum(allValues, [])
	allValues = sorted(allValues)

	return (
		allValues[0],
		allValues[-1],
		allValues[-1] - allValues[0],
	)


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
	return go.Bar(
		name=name,
		x=xData,
		y=[ statistics.median(yDataset[x]) for x in xData ], # if x != 'floyd-warshall'
		error_y=dict(
			type='data',
			symmetric=False,
			array=[ yDataset[x][-1] - statistics.median(yDataset[x]) for x in xData ], # if x != 'floyd-warshall'
			arrayminus=[ statistics.median(yDataset[x]) - yDataset[x][0] for x in xData ], # if x != 'floyd-warshall'
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
	'inst',
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
	'inst',
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

