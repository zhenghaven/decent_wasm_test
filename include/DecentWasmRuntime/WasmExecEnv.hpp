// Copyright (c) 2024 Haofan Zheng
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include "WamrUniquePtr.hpp"

#include <memory>

#include <wasm_export.h>

#include "Exception.hpp"
#include "FuncUtils.hpp"
#include "WasmModuleInstance.hpp"


namespace DecentWasmRuntime
{


struct WasmExecEnvDestroy
{
	void operator()(wasm_exec_env_t ptr) noexcept
	{
		wasm_runtime_destroy_exec_env(ptr);
	}
}; // struct WasmExecEnvDestroy


class WasmExecEnv :
	public WamrUniquePtr<
		typename std::remove_pointer<wasm_exec_env_t>::type,
		WasmExecEnvDestroy
	>
{
public: // static members

	using Base = WamrUniquePtr<
		typename std::remove_pointer<wasm_exec_env_t>::type,
		WasmExecEnvDestroy
	>;

	static WasmExecEnv Create(
		std::shared_ptr<WasmModuleInstance> moduleInst,
		uint32_t stackSize
	)
	{
		wasm_exec_env_t ptr = wasm_runtime_create_exec_env(
			moduleInst->get(),
			stackSize
		);

		if (ptr == nullptr)
		{
			throw Exception("Failed to create execution environment");
		}

		return WasmExecEnv(ptr, moduleInst);
	}

public:

	WasmExecEnv(
		wasm_exec_env_t ptr,
		std::shared_ptr<WasmModuleInstance> moduleInst
	) noexcept :
		Base(ptr), // base constructor is noexcept
		m_moduleInst(moduleInst) // shared_ptr copy is noexcept
	{}

	/**
	 * @brief Copy is prohibited.
	 *
	 */
	WasmExecEnv(const WasmExecEnv&) = delete;

	/**
	 * @brief Construct a new WASM Execution Environment object by moving.
	 *
	 * @param other Another WasmExecEnv object.
	 */
	WasmExecEnv(WasmExecEnv&& other) noexcept :
		Base(std::move(other)), // base move is noexcept
		m_moduleInst(std::move(other.m_moduleInst)) // shared_ptr move is noexcept
	{}

	virtual ~WasmExecEnv()
	{
		// wasm_runtime_destroy_exec_env(get());
	}

	/**
	 * @brief Copy is prohibited.
	 *
	 */
	WasmExecEnv& operator=(const WasmExecEnv&) = delete;

	/**
	 * @brief Move assignment operator.
	 *
	 * @param other The other WasmExecEnv object.
	 * @return This WasmExecEnv object after moving.
	 */
	WasmExecEnv& operator=(WasmExecEnv&& other)
	{
		Base::operator=(std::move(other));
		m_moduleInst = std::move(other.m_moduleInst); // shared_ptr move is noexcept
		return *this;
	}

	template<typename _RetTuple, typename... _Args>
	inline _RetTuple ExecFunc(
		const std::string& funcName,
		_Args&&... args
	)
	{
		static constexpr size_t sk_numRes = std::tuple_size<_RetTuple>::value;

		_RetTuple retVals;
		wasm_val_t wasmRes[sk_numRes + 1];

		auto wasmArg = ToWasmValVector(std::forward<_Args>(args)...);

		wasm_module_inst_t   moduleInst = wasm_runtime_get_module_inst(get());
		wasm_function_inst_t targetFunc =
			wasm_runtime_lookup_function(moduleInst, funcName.c_str(), nullptr);
		if (targetFunc == nullptr)
		{
			throw Exception(
				"Could not find the function named " +
				funcName +
				" in the given WASM module"
			);
		}

		bool execRes = wasm_runtime_call_wasm_a(
			get(), targetFunc,
			static_cast<uint32_t>(sk_numRes), wasmRes,
			static_cast<uint32_t>(wasmArg.size()), wasmArg.data()
		);

		if (!execRes)
		{
			throw Exception(wasm_runtime_get_exception(moduleInst));
		}

		WasmValListToTuple<_RetTuple, sk_numRes>::Read(retVals, wasmRes);

		return retVals;
	}

private:

	std::shared_ptr<WasmModuleInstance> m_moduleInst;

}; // class WasmExecEnv


} // namespace DecentWasmRuntime

