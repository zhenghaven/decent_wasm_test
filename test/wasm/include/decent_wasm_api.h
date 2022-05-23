// Copyright (c) 2022 Haofan Zheng
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#ifndef DECENT_WASM_API_HEADER
#define DECENT_WASM_API_HEADER

#include <stdlib.h>

// ========================================
// Decent WASM native symbols
// ========================================

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void decent_wasm_exit(int status);

int decent_wasm_sum(int a, int b);
void decent_wasm_print(const char * msg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DECENT_WASM_API_HEADER */
