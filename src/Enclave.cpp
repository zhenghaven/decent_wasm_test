#include <cstdint>
#include <cstring>

#include <vector>

#include <sgx_edger8r.h>

#include <wasm_export.h>

#include <DecentWasmWat/WasmWat.h>

#include "DecentWasmRtSupport.hpp"

static char global_heap_buf[10 * 1024 * 1024] = { 0 };

extern "C" {

typedef void (*os_print_function_t)(const char *message);
extern void wasm_os_set_print_function(os_print_function_t pf);
extern sgx_status_t ocall_print(const char* str);

void enclave_print(const char *message)
{
	ocall_print(message);
}

extern void decent_wasm_reg_natives();
extern void decent_wasi_reg_natives();

} // extern "C"

void InitWasmRuntime()
{
	wasm_os_set_print_function(enclave_print);

	RuntimeInitArgs init_args;

	memset(&init_args, 0, sizeof(RuntimeInitArgs));

	init_args.mem_alloc_type = Alloc_With_Pool;
	init_args.mem_alloc_option.pool.heap_buf = global_heap_buf;
	init_args.mem_alloc_option.pool.heap_size = sizeof(global_heap_buf);

	/* initialize runtime environment */
	if (!wasm_runtime_full_init(&init_args))
	{
		throw std::runtime_error("Init runtime environment failed");
	}

	decent_wasi_reg_natives();
	decent_wasm_reg_natives();
}

std::tuple<uint32_t, uint64_t> ExecuteDecentWasmMain(
	const std::vector<uint8_t>& wasmBuf,
	const std::vector<uint8_t>& eventId,
	const std::vector<uint8_t>& msgCont,
	uint64_t threshold)
{
	using MainRetType = std::tuple<int32_t>;
	using CtrRetType = std::tuple<int64_t>;
	static constexpr uint32_t stackSize = 16 * 1024; // 16 KB
	static constexpr uint32_t heapSize  = 16 * 1024; // 16 KB

	if (eventId.size() > std::numeric_limits<uint32_t>::max() ||
		msgCont.size() > std::numeric_limits<uint32_t>::max())
	{
		throw std::invalid_argument("The message received is too large");
	}

	Decent::WasmRt::WasmModule mod =
		Decent::WasmRt::WasmModule::Load(wasmBuf);

	Decent::WasmRt::WasmModuleInstance inst =
		mod.Instantiate(stackSize, heapSize);

	Decent::WasmRt::WasmExecEnv execEnv =
		inst.CreateExecEnv(stackSize);

	// Decent::WasmRt::WasmModMem<uint64_t> testMem =
	// 	inst.Malloc<uint64_t>();

	Decent::WasmRt::WasmModMem<uint8_t[]> eventIdMem =
		inst.Malloc<uint8_t[]>(eventId.size());
	std::copy(eventId.begin(), eventId.end(), eventIdMem.get());

	Decent::WasmRt::WasmModMem<uint8_t[]> msgContMem =
		inst.Malloc<uint8_t[]>(msgCont.size());
	std::copy(msgCont.begin(), msgCont.end(), msgContMem.get());

	auto mainRetVals = execEnv.ExecFunc<MainRetType>("decent_wasm_injected_main",
		eventIdMem, eventIdMem.size(), msgContMem, msgContMem.size(), threshold);

	auto ictrRetVals = execEnv.ExecFunc<CtrRetType>("decent_wasm_get_icounter");

	return std::make_tuple(
		std::get<0>(mainRetVals),
		std::get<0>(ictrRetVals));
}

void ExecuteWasm(const uint8_t *wasm_file_buf, size_t wasm_file_size)
{
	wasm_module_t wasm_module = NULL;
	wasm_module_inst_t wasm_module_inst = NULL;
	char error_buf[128];
	const char *exception;

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
	try
	{
		InitWasmRuntime();

		std::vector<uint8_t> wasm(wasm_file_buf, wasm_file_buf + wasm_file_size);

		// std::string wat = DecentWasmWat::Wasm2Wat(
		// 	"filename.wasm", wasm, DecentWasmWat::Wasm2WatConfig());

		std::vector<uint8_t> wasmFromWat = wasm;
			// DecentWasmWat::Wat2Wasm(
			// "filename.wat", wat, DecentWasmWat::Wat2WasmConfig());

		// ExecuteWasm(wasmFromWat.data(), wasmFromWat.size());

		std::vector<uint8_t> eventId = { 12U, 34U, 56U, 78U, 90U, };
		std::vector<uint8_t> msgCont = { 98U, 76U, 54U, 32U, 10U, };
		uint64_t threshold = 1000;

		uint32_t mainRet = 0;
		uint64_t ictrRet = 0;
		std::tie(mainRet, ictrRet) =
			ExecuteDecentWasmMain(wasmFromWat, eventId, msgCont, threshold);

		std::string retMsg;
		retMsg = "WASM code returned: " + std::to_string(mainRet) + "\n";
		enclave_print(retMsg.c_str());
		retMsg = "WASM icounter val:  " + std::to_string(ictrRet) + "\n";
		enclave_print(retMsg.c_str());
	}
	catch(const std::exception& e)
	{
		ocall_print(e.what());
		ocall_print("\n");
	}
}

} // extern "C"
