// Copyright (c) 2024 Haofan Zheng
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstdint>
#include <cstring>

#include <vector>

#include <DecentWasmRuntime/Internal/make_unique.hpp>
#include <DecentWasmRuntime/MainRunner.hpp>
#include <DecentWasmRuntime/WasmRuntimeStaticHeap.hpp>

#include "SystemIO.hpp"


inline bool DecentWasmMain(
	const uint8_t *wasm_file, size_t wasm_file_size,
	const uint8_t *wasm_nopt_file, size_t wasm_nopt_file_size
)
{
	using namespace DecentWasmRuntime;
	try
	{
		std::vector<uint8_t> wasmBytecode(
			wasm_file,
			wasm_file + wasm_file_size
		);
		std::vector<uint8_t> instWasmBytecode(
			wasm_nopt_file,
			wasm_nopt_file + wasm_nopt_file_size
		);

		auto wasmRt = SharedWasmRuntime(
			Internal::make_unique<WasmRuntimeStaticHeap>(
				PrintCStr,
				24 * 1024 * 1024 // 24 MB
			)
		);

		std::vector<uint8_t> eventId = {
			'D', 'e', 'c', 'e', 'n', 't', '\0'
		};
		std::vector<uint8_t> msgContent = {
			'E', 'v', 'e', 'n', 't', 'M', 'e', 's', 's', 'a', 'g', 'e', '\0'
		};
		uint64_t threshold = std::numeric_limits<uint64_t>::max() / 2;

		{
			MainRunner(
				wasmRt,
				wasmBytecode,
				eventId,
				msgContent,
				1 * 1024 * 1024,  // mod stack:  1 MB
				16 * 1024 * 1024, // mod heap:  16 MB
				1 * 1024 * 1024   // exec stack: 1 MB
			).RunPlain();
		}

		{
			MainRunner(
				wasmRt,
				instWasmBytecode,
				eventId,
				msgContent,
				1 * 1024 * 1024,  // mod stack:  1 MB
				16 * 1024 * 1024, // mod heap:  16 MB
				1 * 1024 * 1024   // exec stack: 1 MB
			).RunInstrumented(threshold);
		}

		return true;
	}
	catch(const std::exception& e)
	{
		PrintStr(e.what());
		PrintStr("\n");
		return false;
	}
}

