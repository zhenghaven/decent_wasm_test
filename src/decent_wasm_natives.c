// Copyright (c) 2024 Haofan Zheng
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.


#include <stddef.h>

#include <wasm_export.h>

extern void enclave_print(const char *message);
extern int decent_wasm_sum(wasm_exec_env_t exec_env , int a, int b);
extern void decent_wasm_print_string(wasm_exec_env_t exec_env, const char * msg);
extern void decent_wasm_start_benchmark(wasm_exec_env_t exec_env);
extern void decent_wasm_stop_benchmark(wasm_exec_env_t exec_env);
extern void decent_wasm_exit(wasm_exec_env_t exec_env, int exit_code);
extern void decent_wasm_counter_exceed(wasm_exec_env_t exec_env);
extern uint32_t decent_wasm_get_event_id_len(wasm_exec_env_t exec_env);
extern uint32_t decent_wasm_get_event_data_len(wasm_exec_env_t exec_env);
extern uint32_t decent_wasm_get_event_id(wasm_exec_env_t exec_env, void* wasmPtr, uint32_t len);
extern uint32_t decent_wasm_get_event_data(wasm_exec_env_t exec_env, uint32_t wasmPtr, uint32_t len);


static NativeSymbol gs_DecentWasmNatives[] =
{
	{
		"decent_wasm_sum", // WASM function name
		decent_wasm_sum,   // the native function pointer
		"(ii)i",           // the function prototype signature
		NULL,
	},
	{
		"decent_wasm_print", // WASM function name
		decent_wasm_print_string,   // the native function pointer
		"($)",               // the function prototype signature
		NULL,
	},
	{
		"decent_wasm_print_string", // WASM function name
		decent_wasm_print_string,   // the native function pointer
		"($)",               // the function prototype signature
		NULL,
	},
	{
		"decent_wasm_start_benchmark", // WASM function name
		decent_wasm_start_benchmark,   // the native function pointer
		"()",               // the function prototype signature
		NULL,
	},
	{
		"decent_wasm_stop_benchmark", // WASM function name
		decent_wasm_stop_benchmark,   // the native function pointer
		"()",               // the function prototype signature
		NULL,
	},
	{
		"decent_wasm_get_event_id_len", // WASM function name
		decent_wasm_get_event_id_len,   // the native function pointer
		"()i",               // the function prototype signature
		NULL,
	},
	{
		"decent_wasm_get_event_data_len", // WASM function name
		decent_wasm_get_event_data_len,   // the native function pointer
		"()i",               // the function prototype signature
		NULL,
	},
	{
		"decent_wasm_get_event_id", // WASM function name
		decent_wasm_get_event_id,   // the native function pointer
		"(*~)i",               // the function prototype signature
		NULL,
	},
	{
		"decent_wasm_get_event_data", // WASM function name
		decent_wasm_get_event_data,   // the native function pointer
		"(*~)i",               // the function prototype signature
		NULL,
	},
	{
		"decent_wasm_exit", // WASM function name
		decent_wasm_exit,   // the native function pointer
		"(i)",               // the function prototype signature
		NULL,
	},
	{
		"decent_wasm_counter_exceed", // WASM function name
		decent_wasm_counter_exceed,   // the native function pointer
		"()",               // the function prototype signature
		NULL,
	},
};


// Register WASM native functions
// Ref: https://github.com/bytecodealliance/wasm-micro-runtime/blob/main/doc/export_native_api.md


void decent_wasm_reg_natives()
{
	const int symNum = sizeof(gs_DecentWasmNatives) / sizeof(NativeSymbol);
	if (!wasm_runtime_register_natives("env",
		gs_DecentWasmNatives, symNum))
	{
		enclave_print("ERROR: Failed to register Decent WASM native symbols!");
	}
}
