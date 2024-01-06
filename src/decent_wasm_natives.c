#include <stddef.h>

#include <wasm_export.h>

// Register WASM native functions
// Ref: https://github.com/bytecodealliance/wasm-micro-runtime/blob/main/doc/export_native_api.md

extern void enclave_print(const char *message);

int decent_wasm_sum(wasm_exec_env_t exec_env , int a, int b)
{
	(void)exec_env;
	return a + b;
}

void decent_wasm_print_string(wasm_exec_env_t exec_env, const char * msg)
{
	(void)exec_env;
	enclave_print(msg);
}

void decent_wasm_start_benchmark(wasm_exec_env_t exec_env)
{
	(void)exec_env;
	enclave_print("Benchmark started.\n");
}

void decent_wasm_stop_benchmark(wasm_exec_env_t exec_env)
{
	(void)exec_env;
	enclave_print("Benchmark stopped.\n");
}

void decent_wasm_exit(wasm_exec_env_t exec_env, int exit_code)
{
	(void)exec_env;
	(void)exit_code;
    wasm_module_inst_t module_inst = wasm_runtime_get_module_inst(exec_env);
    /* Here throwing exception is just to let wasm app exit,
       the upper layer should clear the exception and return
       as normal */
    wasm_runtime_set_exception(module_inst, "decent wasm exit");
}

void decent_wasm_counter_exceed(wasm_exec_env_t exec_env)
{
	(void)exec_env;
    wasm_module_inst_t module_inst = wasm_runtime_get_module_inst(exec_env);
    /* Here throwing exception is just to let wasm app exit,
       the upper layer should clear the exception and return
       as normal */
    wasm_runtime_set_exception(module_inst, "decent counter exceed");
}

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

void decent_wasm_reg_natives()
{
	const int symNum = sizeof(gs_DecentWasmNatives) / sizeof(NativeSymbol);
	if (!wasm_runtime_register_natives("env",
		gs_DecentWasmNatives, symNum))
	{
		enclave_print("ERROR: Failed to register Decent WASM native symbols!");
	}
}
