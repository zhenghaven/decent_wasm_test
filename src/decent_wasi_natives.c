#include <stddef.h>
#include <stdint.h>

#include <wasm_export.h>


extern int wasm_os_printf(const char *message, ...);

static void
wasi_proc_exit(wasm_exec_env_t exec_env, uint32_t rval)
{
    (void)rval;
    wasm_module_inst_t module_inst = wasm_runtime_get_module_inst(exec_env);
    /* Here throwing exception is just to let wasm app exit,
       the upper layer should clear the exception and return
       as normal */
    wasm_runtime_set_exception(module_inst, "wasi proc exit");
}

static NativeSymbol gs_DecentWasiNatives[] =
{
	{
		"proc_exit",
		wasi_proc_exit,
		"(i)",
		NULL,
	},
};

void decent_wasi_reg_natives()
{
	const int symNum = sizeof(gs_DecentWasiNatives) / sizeof(NativeSymbol);
	if (!wasm_runtime_register_natives("wasi_snapshot_preview1",
		gs_DecentWasiNatives, symNum))
	{
		wasm_os_printf("ERROR: Failed to register Decent WASI native symbols!");
	}
}
