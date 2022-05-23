// Copyright (c) 2022 Haofan Zheng
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#ifndef DECENT_WASM_IMPL_HEADER
#define DECENT_WASM_IMPL_HEADER

#include <wasi/api.h>

#include "decent_wasm_api.h"

// ========================================
// helper functions
// ========================================

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void decent_wasm_exit(int status)
{
	__wasi_proc_exit(status);
}

void decent_wasm_prerequisite_imports(void)
{
	decent_wasm_exit(0);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DECENT_WASM_IMPL_HEADER */
