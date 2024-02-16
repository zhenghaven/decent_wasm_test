// Copyright (c) 2024 Haofan Zheng
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstdint>

#include <tuple>
#include <vector>

#include "ExecEnvUserData.hpp"
#include "SharedWasmExecEnv.hpp"
#include "SharedWasmModule.hpp"
#include "SharedWasmModuleInstance.hpp"
#include "SharedWasmRuntime.hpp"


namespace DecentWasmRuntime
{


class MainRunner
{
public: // static members:

	static const std::string& sk_globalCounterName()
	{
		static const std::string sk_globalCounterName = "decent_wasm_counter";
		return sk_globalCounterName;
	}

	static const std::string& sk_globalThresholdName()
	{
		static const std::string sk_globalThresholdName = "decent_wasm_threshold";
		return sk_globalThresholdName;
	}

public:
	MainRunner(
		SharedWasmRuntime& wasmRt,
		const std::vector<uint8_t>& wasmBytecode,
		const std::vector<uint8_t>& eventId,
		const std::vector<uint8_t>& msgContent,
		uint32_t modStackSize,
		uint32_t modHeapSize,
		uint32_t execStackSize
	) :
		m_printFunc(wasmRt->GetPrintFunc()),
		m_module(wasmRt.LoadModule(wasmBytecode)),
		m_modInst(m_module.Instantiate(modStackSize, modHeapSize)),
		m_execEnv(m_modInst.CreateExecEnv(execStackSize))
	{
		std::unique_ptr<ExecEnvUserData> execEnvUserData =
			Internal::make_unique<ExecEnvUserData>();
		execEnvUserData->SetEventId(eventId);
		execEnvUserData->SetEventData(msgContent);
		m_execEnv->SetUserData(std::move(execEnvUserData));
	}

	int32_t RunPlain()
	{
		using MainRetType = std::tuple<int32_t>;

		auto mainRetVals = m_execEnv->ExecFunc<MainRetType>(
			"decent_wasm_main",
			static_cast<uint32_t>(m_execEnv->GetUserData().GetEventId().size()),
			static_cast<uint32_t>(m_execEnv->GetUserData().GetEventData().size())
		);

		return std::get<0>(mainRetVals);
	}

	int32_t RunInstrumented(uint64_t threshold)
	{
		using MainRetType = std::tuple<int32_t>;

		m_threshold = threshold;

		auto mainRetVals = m_execEnv->ExecFunc<MainRetType>(
			"decent_wasm_injected_main",
			static_cast<uint32_t>(m_execEnv->GetUserData().GetEventId().size()),
			static_cast<uint32_t>(m_execEnv->GetUserData().GetEventData().size()),
			static_cast<uint64_t>(threshold)
		);

		m_counter = m_modInst->GetGlobal<uint64_t>(sk_globalCounterName());
		m_printFunc((
			"Threshold: " + std::to_string(m_threshold) + ", "
			"Counter: "   + std::to_string(m_counter) + "\n"
		).c_str());

		return std::get<0>(mainRetVals);
	}

	uint64_t GetThreshold() const noexcept
	{
		return m_threshold;
	}

	uint64_t GetCounter() const noexcept
	{
		return m_counter;
	}

	void ResetThresholdAndCounter()
	{
		m_modInst->SetGlobal<uint64_t>(sk_globalCounterName(), 0);
		m_modInst->SetGlobal<uint64_t>(sk_globalThresholdName(), 0);
	}

private:

	os_print_function_t m_printFunc;
	SharedWasmModule m_module;
	SharedWasmModuleInstance m_modInst;
	SharedWasmExecEnv m_execEnv;

	uint64_t m_threshold = 0;
	uint64_t m_counter = 0;
}; // class MainRunner


} // namespace DecentWasmRuntime

