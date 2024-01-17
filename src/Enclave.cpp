// Copyright (c) 2024 Haofan Zheng
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#include "DecentMain.hpp"


extern "C" {

void ecall_decent_wasm_main(
	uint8_t *wasm_file, size_t wasm_file_size,
	uint8_t *wasm_inst_file, size_t wasm_inst_file_size
)
{
	DecentWasmMain(
		wasm_file, wasm_file_size,
		wasm_inst_file, wasm_inst_file_size
	);
}

} // extern "C"
