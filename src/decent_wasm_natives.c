#include <stddef.h>

#include <wasm_export.h>

// Register WASM native functions
// Ref: https://github.com/bytecodealliance/wasm-micro-runtime/blob/main/doc/export_native_api.md

extern void enclave_print(const char *message);

int decent_wasm_sum(wasm_exec_env_t exec_env , int a, int b)
{
	return a + b;
}

void decent_wasm_print(wasm_exec_env_t exec_env, char * msg)
{
	enclave_print(msg);
}

static NativeSymbol s_DecentWasmNatives[] =
{
	{
		"decent_wasm_sum",            // WASM function name
		decent_wasm_sum,  // the native function pointer
		"(ii)i",          // the function prototype signature
		NULL,
	},
	{
		"decent_wasm_print",            // WASM function name
		decent_wasm_print,  // the native function pointer
		"($)",              // the function prototype signature
		NULL,
	}
};

void decent_wasm_reg_natives()
{
	const int symNum = sizeof(s_DecentWasmNatives) / sizeof(NativeSymbol);
	if (!wasm_runtime_register_natives("env",
		s_DecentWasmNatives, symNum))
	{
		enclave_print("ERROR: Failed to register Decent WASM native symbols!");
	}
}
