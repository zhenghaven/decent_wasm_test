#include <cstdint>
#include <cstring>

#include <vector>

#include <sgx_edger8r.h>

#include <wasm_export.h>

#include <DecentWasmWat/WasmWat.h>

#include <DecentWasmRuntime/ExecEnvUserData.hpp>
#include <DecentWasmRuntime/Internal/make_unique.hpp>
#include <DecentWasmRuntime/WasmRuntimeStaticHeap.hpp>
#include <DecentWasmRuntime/SharedWasmRuntime.hpp>

extern "C" sgx_status_t ocall_print(const char* str);

extern "C" void enclave_print(const char* str)
{
	ocall_print(str);
}


static inline void EnclavePrint(const std::string& str)
{
	enclave_print(str.c_str());
}


static inline int32_t ExecDecentInstrumentedWasm(
	DecentWasmRuntime::SharedWasmRuntime& wasmRt,
	const std::vector<uint8_t>& wasmBytecode,
	const std::vector<uint8_t>& eventId,
	const std::vector<uint8_t>& msgContent,
	uint64_t threshold
)
{
	using namespace DecentWasmRuntime;

	using MainRetType = std::tuple<int32_t>;

	auto wasmModule = wasmRt.LoadModule(wasmBytecode);

	auto wasmInst = wasmModule.Instantiate(
		1 * 1024 * 1024, // stack size
		8 * 1024 * 1024 // heap size
	);
	auto execEnv = wasmInst.CreateExecEnv(512 * 1024); // stack size

	std::unique_ptr<ExecEnvUserData> execEnvUserData =
		Internal::make_unique<ExecEnvUserData>();
	execEnvUserData->SetEventId(eventId);
	execEnvUserData->SetEventData(msgContent);
	execEnv->SetUserData(std::move(execEnvUserData));
	// NOTE: !! execEnvUserData is invalid after this point !!

	auto mainRetVals = execEnv->ExecFunc<MainRetType>(
		"decent_wasm_injected_main",
		static_cast<uint32_t>(eventId.size()),
		static_cast<uint32_t>(msgContent.size()),
		static_cast<uint64_t>(threshold)
	);

	return std::get<0>(mainRetVals);
}


static inline int32_t ExecDecentPlainWasm(
	DecentWasmRuntime::SharedWasmRuntime& wasmRt,
	const std::vector<uint8_t>& wasmBytecode,
	const std::vector<uint8_t>& eventId,
	const std::vector<uint8_t>& msgContent
)
{
	using namespace DecentWasmRuntime;

	using MainRetType = std::tuple<int32_t>;

	auto wasmModule = wasmRt.LoadModule(wasmBytecode);

	auto wasmInst = wasmModule.Instantiate(
		1 * 1024 * 1024, // stack size
		8 * 1024 * 1024 // heap size
	);
	auto execEnv = wasmInst.CreateExecEnv(512 * 1024); // stack size

	std::unique_ptr<ExecEnvUserData> execEnvUserData =
		Internal::make_unique<ExecEnvUserData>();
	execEnvUserData->SetEventId(eventId);
	execEnvUserData->SetEventData(msgContent);
	execEnv->SetUserData(std::move(execEnvUserData));
	// NOTE: !! execEnvUserData is invalid after this point !!

	auto mainRetVals = execEnv->ExecFunc<MainRetType>(
		"decent_wasm_main",
		static_cast<uint32_t>(eventId.size()),
		static_cast<uint32_t>(msgContent.size())
	);

	return std::get<0>(mainRetVals);
}


extern "C" {

void ecall_iwasm_main(
	uint8_t *wasm_file, size_t wasm_file_size,
	uint8_t *wasm_inst_file, size_t wasm_inst_file_size
)
{
	using namespace DecentWasmRuntime;
	try
	{
		auto wasmRt = SharedWasmRuntime(
			Internal::make_unique<WasmRuntimeStaticHeap>(
				enclave_print,
				16 * 1024 * 1024 // 16 MB
			)
		);

		std::vector<uint8_t> eventId = {
			'D', 'e', 'c', 'e', 'n', 't', '\0'
		};
		std::vector<uint8_t> msgContent = {
			'E', 'v', 'e', 'n', 't', 'M', 'e', 's', 's', 'a', 'g', 'e', '\0'
		};
		uint64_t threshold = 1000000000;

		std::vector<uint8_t> wasmBytecode(
			wasm_file,
			wasm_file + wasm_file_size
		);
		ExecDecentPlainWasm(
			wasmRt,
			wasmBytecode,
			eventId,
			msgContent
		);

		std::vector<uint8_t> instWasmBytecode(
			wasm_inst_file,
			wasm_inst_file + wasm_inst_file_size
		);
		ExecDecentInstrumentedWasm(
			wasmRt,
			instWasmBytecode,
			eventId,
			msgContent,
			threshold
		);
	}
	catch(const std::exception& e)
	{
		ocall_print(e.what());
		ocall_print("\n");
	}
}

} // extern "C"
