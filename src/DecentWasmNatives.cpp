// Copyright (c) 2024 Haofan Zheng
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.


#include <wasm_export.h>

#include <DecentWasmRuntime/WasmExecEnv.hpp>



extern "C" void enclave_print(const char *message);


extern "C" int decent_wasm_sum(wasm_exec_env_t exec_env , int a, int b)
{
	(void)exec_env;
	return a + b;
}


extern "C" void decent_wasm_print_string(wasm_exec_env_t exec_env, const char * msg)
{
	(void)exec_env;
	enclave_print(msg);
}


extern "C" void decent_wasm_start_benchmark(wasm_exec_env_t exec_env)
{
	(void)exec_env;
	enclave_print("Benchmark started.\n");
}


extern "C" void decent_wasm_stop_benchmark(wasm_exec_env_t exec_env)
{
	(void)exec_env;
	enclave_print("Benchmark stopped.\n");
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
	(void)exec_env;
	(void)exit_code;
	wasm_module_inst_t module_inst = wasm_runtime_get_module_inst(exec_env);
	/* Here throwing exception is just to let wasm app exit,
		the upper layer should clear the exception and return
		as normal */
    wasm_runtime_set_exception(module_inst, "decent wasm exit");
}


extern "C" void decent_wasm_counter_exceed(wasm_exec_env_t exec_env)
{
	(void)exec_env;
    wasm_module_inst_t module_inst = wasm_runtime_get_module_inst(exec_env);
    /* Here throwing exception is just to let wasm app exit,
       the upper layer should clear the exception and return
       as normal */
    wasm_runtime_set_exception(module_inst, "decent counter exceed");
}

