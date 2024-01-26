// Copyright (c) 2024 Haofan Zheng
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.


#include <cstdint>

#include <wasm_export.h>

#include <DecentWasmRuntime/WasmExecEnv.hpp>

#include "SystemIO.hpp"


extern "C" void emscripten_memcpy_js(
	wasm_exec_env_t exec_env,
	void* dest,
	const void* src,
	size_t n
)
{
	(void)exec_env;
	std::memcpy(dest, src, n);
}

extern "C" int decent_wasm_sum(wasm_exec_env_t exec_env, int a, int b)
{
	(void)exec_env;
	return a + b;
}


extern "C" void decent_wasm_print_string(wasm_exec_env_t exec_env, const char * msg)
{
	(void)exec_env;
	PrintStr(msg);
}


extern "C" void decent_wasm_start_benchmark(wasm_exec_env_t exec_env)
{
	using namespace DecentWasmRuntime;

	PrintStr("Benchmark started.\n");

	try
	{
		uint64_t startUs = GetTimestampUs();

		WasmExecEnv::FromUserData(exec_env).GetUserData().SetStartTime(startUs);
	}
	catch (const std::exception& e)
	{
		wasm_module_inst_t module_inst = wasm_runtime_get_module_inst(exec_env);
		wasm_runtime_set_exception(module_inst, e.what());
	}
}


extern "C" void decent_wasm_stop_benchmark(wasm_exec_env_t exec_env)
{
	using namespace DecentWasmRuntime;

	try
	{
		uint64_t endUs = GetTimestampUs();

		const auto& execEnv = WasmExecEnv::FromConstUserData(exec_env);
		uint64_t startUs = execEnv.GetUserData().GetStartTime();

		uint64_t durationUs = endUs - startUs;
		std::string msg =
			"Benchmark stopped. (Started @ " + std::to_string(startUs) + " us,"
			" ended @ " + std::to_string(endUs) + " us, "
			"spent " + std::to_string(durationUs) + " us)\n";
		PrintStr(msg);
	}
	catch (const std::exception& e)
	{
		wasm_module_inst_t module_inst = wasm_runtime_get_module_inst(exec_env);
		wasm_runtime_set_exception(module_inst, e.what());
	}
}


extern "C" uint32_t decent_wasm_get_event_id_len(wasm_exec_env_t exec_env)
{
	using namespace DecentWasmRuntime;
	return WasmExecEnv::FromConstUserData(exec_env).GetUserData().GetEventId().size();
}


extern "C" uint32_t decent_wasm_get_event_data_len(wasm_exec_env_t exec_env)
{
	using namespace DecentWasmRuntime;
	return WasmExecEnv::FromConstUserData(exec_env).GetUserData().GetEventData().size();
}


extern "C" uint32_t decent_wasm_get_event_id(
	wasm_exec_env_t exec_env,
	void* nativePtr,
	uint32_t len
)
{
	using namespace DecentWasmRuntime;

	uint8_t* ptr = static_cast<uint8_t*>(nativePtr);

	const auto& eventId = WasmExecEnv::FromConstUserData(exec_env).GetUserData().GetEventId();
	size_t cpSize = len <= eventId.size() ? len : eventId.size();

	std::copy(eventId.begin(), eventId.begin() + cpSize, ptr);

	return eventId.size();
}


extern "C" uint32_t decent_wasm_get_event_data(
	wasm_exec_env_t exec_env,
	void* nativePtr,
	uint32_t len
)
{
	using namespace DecentWasmRuntime;

	uint8_t* ptr = static_cast<uint8_t*>(nativePtr);

	const auto& eventData = WasmExecEnv::FromConstUserData(exec_env).GetUserData().GetEventData();
	size_t cpSize = len <= eventData.size() ? len : eventData.size();

	std::copy(eventData.begin(), eventData.begin() + cpSize, ptr);

	return WasmExecEnv::FromConstUserData(exec_env).GetUserData().GetEventData().size();
}


extern "C" void decent_wasm_exit(wasm_exec_env_t exec_env, int exit_code)
{
	(void)exit_code;
	wasm_module_inst_t module_inst = wasm_runtime_get_module_inst(exec_env);
	/* Here throwing exception is just to let wasm app exit,
		the upper layer should clear the exception and return
		as normal */
	wasm_runtime_set_exception(module_inst, "decent wasm exit");
}


extern "C" void decent_wasm_counter_exceed(wasm_exec_env_t exec_env)
{
	using namespace DecentWasmRuntime;

	static const std::string sk_globalThresholdName = "decent_wasm_threshold";
	static const std::string sk_globalCounterName = "decent_wasm_counter";

	try
	{
		uint64_t threshold = WasmExecEnv::FromConstUserData(exec_env).GetModuleInstance().
			GetGlobal<uint64_t>(sk_globalThresholdName);
		uint64_t counter = WasmExecEnv::FromConstUserData(exec_env).GetModuleInstance().
			GetGlobal<uint64_t>(sk_globalCounterName);
		wasm_module_inst_t module_inst = wasm_runtime_get_module_inst(exec_env);

		std::string msg = "decent counter exceed. ( "
			"Threshold: " + std::to_string(threshold) + ", "
			"Counter: " + std::to_string(counter) + ")" ;
		PrintStr(msg);

		/* Here throwing exception is just to let wasm app exit,
		the upper layer should clear the exception and return
		as normal */
		wasm_runtime_set_exception(module_inst, "decent counter exceed");
	}
	catch (const std::exception& e)
	{
		wasm_module_inst_t module_inst = wasm_runtime_get_module_inst(exec_env);
		wasm_runtime_set_exception(module_inst, e.what());
	}
}

