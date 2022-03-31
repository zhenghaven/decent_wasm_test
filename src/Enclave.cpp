#include <cstdint>
#include <cstring>

#include <vector>

#include <sgx_edger8r.h>

#include <wasm_export.h>

#include <DecentWasmWat/WasmWat.h>

static char global_heap_buf[10 * 1024 * 1024] = { 0 };

extern "C" {

typedef void (*os_print_function_t)(const char *message);
extern void wasm_os_set_print_function(os_print_function_t pf);
extern sgx_status_t ocall_print(const char* str);

} // extern "C"

void enclave_print(const char *message)
{
	ocall_print(message);
}

void ExecuteWasm(const uint8_t *wasm_file_buf, size_t wasm_file_size)
{
	wasm_module_t wasm_module = NULL;
	wasm_module_inst_t wasm_module_inst = NULL;
	RuntimeInitArgs init_args;
	char error_buf[128];
	const char *exception;

	wasm_os_set_print_function(enclave_print);

	memset(&init_args, 0, sizeof(RuntimeInitArgs));

	init_args.mem_alloc_type = Alloc_With_Pool;
	init_args.mem_alloc_option.pool.heap_buf = global_heap_buf;
	init_args.mem_alloc_option.pool.heap_size = sizeof(global_heap_buf);

	/* initialize runtime environment */
	if (!wasm_runtime_full_init(&init_args)) {
		ocall_print("Init runtime environment failed.");
		ocall_print("\n");
		return;
	}

	/* load WASM module */
	if (!(wasm_module = wasm_runtime_load(wasm_file_buf, wasm_file_size,
										error_buf, sizeof(error_buf)))) {
		ocall_print(error_buf);
		ocall_print("\n");
		goto fail1;
	}

	/* instantiate the module */
	if (!(wasm_module_inst =
			wasm_runtime_instantiate(wasm_module, 16 * 1024, 16 * 1024,
									error_buf, sizeof(error_buf)))) {
		ocall_print(error_buf);
		ocall_print("\n");
		goto fail2;
	}

	/* execute the main function of wasm app */
	wasm_application_execute_main(wasm_module_inst, 0, NULL);
	if ((exception = wasm_runtime_get_exception(wasm_module_inst))) {
		ocall_print(exception);
		ocall_print("\n");
	}

	/* destroy the module instance */
	wasm_runtime_deinstantiate(wasm_module_inst);

fail2:
	/* unload the module */
	wasm_runtime_unload(wasm_module);

fail1:
	/* destroy runtime environment */
	wasm_runtime_destroy();
}

extern "C" {

void ecall_iwasm_main(uint8_t *wasm_file_buf, size_t wasm_file_size)
{
	std::vector<uint8_t> wasm(wasm_file_buf, wasm_file_buf + wasm_file_size);

	std::string wat = DecentWasmWat::Wasm2Wat(
		"filename.wasm", wasm, DecentWasmWat::Wasm2WatConfig());

	std::vector<uint8_t> wasmFromWat = DecentWasmWat::Wat2Wasm(
		"filename.wat", wat, DecentWasmWat::Wat2WasmConfig());

	ExecuteWasm(wasmFromWat.data(), wasmFromWat.size());
}

} // extern "C"
