#include <cstdint>
#include <cstring>

#include <vector>

#include <sgx_edger8r.h>

#include <wasm_export.h>

#include <DecentWasmWat/WasmWat.h>

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
	using MainRetType = std::tuple<int32_t>;

	if (eventId.size() > static_cast<uint64_t>(std::numeric_limits<int32_t>::max()))
	{
		throw std::invalid_argument("The event ID received is too large");
	}
	if (msgContent.size() > static_cast<uint64_t>(std::numeric_limits<int32_t>::max()))
	{
		throw std::invalid_argument("The message received is too large");
	}
	if (threshold > static_cast<uint64_t>(std::numeric_limits<int64_t>::max()))
	{
		throw std::invalid_argument("The given threshold is over the maximum value");
	}

	auto wasmModule = wasmRt.LoadModule(wasmBytecode);

	auto wasmInst = wasmModule.Instantiate(
		1 * 1024 * 1024, // stack size
		8 * 1024 * 1024 // heap size
	);
	auto execEnv = wasmInst.CreateExecEnv(512 * 1024); // stack size

	auto wasmEvId = wasmInst.NewMem<uint8_t[]>(eventId.size());
	EnclavePrint("wasmEvId @ " + std::to_string(wasmEvId.GetWasmPtr()) + "\n");
	wasmEvId.CopyContainer(eventId);
	auto wasmMsgContent = wasmInst.NewMem<uint8_t[]>(msgContent.size());
	EnclavePrint("wasmMsgContent @ " + std::to_string(wasmMsgContent.GetWasmPtr()) + "\n");
	wasmMsgContent.CopyContainer(msgContent);

	auto mainRetVals = execEnv->ExecFunc<MainRetType>(
		"decent_wasm_injected_main",
		wasmEvId, static_cast<int32_t>(wasmEvId.size()),
		wasmMsgContent, static_cast<int32_t>(wasmMsgContent.size()),
		static_cast<int64_t>(threshold)
	);

	return std::get<0>(mainRetVals);
}


extern "C" {

void ecall_iwasm_main(uint8_t *wasm_file_buf, size_t wasm_file_size)
{
	using namespace DecentWasmRuntime;
	try
	{
		// InitWasmRuntime();

		std::vector<uint8_t> eventId = { 12U, 34U, 56U, 78U, 90U, };
		std::vector<uint8_t> msgContent = { 98U, 76U, 54U, 32U, 10U, };
		uint64_t threshold = 1000000000;

		std::vector<uint8_t> wasmBytecode(
			wasm_file_buf,
			wasm_file_buf + wasm_file_size
		);

		auto wasmRt = SharedWasmRuntime(
			Internal::make_unique<WasmRuntimeStaticHeap>(
				enclave_print,
				16 * 1024 * 1024 // 16 MB
			)
		);

		ExecDecentInstrumentedWasm(
			wasmRt,
			wasmBytecode,
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
