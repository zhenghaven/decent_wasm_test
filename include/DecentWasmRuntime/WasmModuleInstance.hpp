// Copyright (c) 2024 Haofan Zheng
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include "WamrUniquePtr.hpp"

#include <memory>

#include <wasm_export.h>

#include "Exception.hpp"
#include "WasmModule.hpp"


namespace DecentWasmRuntime
{


struct WasmModuleDeinstantiate
{
	void operator()(wasm_module_inst_t ptr) noexcept
	{
		wasm_runtime_deinstantiate(ptr);
	}
}; // struct WasmModuleDeinstantiate


class WasmModuleInstance :
	public WamrUniquePtr<
		typename std::remove_pointer<wasm_module_inst_t>::type,
		WasmModuleDeinstantiate
	>
{
public: // static members

	using Base = WamrUniquePtr<
		typename std::remove_pointer<wasm_module_inst_t>::type,
		WasmModuleDeinstantiate
	>;

	friend class WasmExecEnv;

	template<typename _ValType>
	friend class InstMemPtrBase;

	static  WasmModuleInstance Instantiate(
		std::shared_ptr<WasmModule> module,
		uint32_t stackSize,
		uint32_t heapSize
	)
	{
		char errorBuf[512];

		wasm_module_inst_t ptr = wasm_runtime_instantiate(
			module->get(),
			stackSize,
			heapSize,
			errorBuf,
			sizeof(errorBuf)
		);

		if (ptr == nullptr)
		{
			throw Exception(errorBuf);
		}

		return WasmModuleInstance(ptr, module);
	}

public:

	WasmModuleInstance(
		wasm_module_inst_t ptr,
		std::shared_ptr<WasmModule> module
	) noexcept :
		Base(ptr), // base constructor is noexcept
		m_module(module) // shared_ptr copy is noexcept
	{}

	/**
	 * @brief Copy is prohibited.
	 *
	 */
	WasmModuleInstance(const WasmModuleInstance&) = delete;

	/**
	 * @brief Construct a new WASM module instance object by moving.
	 *
	 * @param other Another WASM module instance object.
	 */
	WasmModuleInstance(WasmModuleInstance&& other) noexcept :
		Base(std::forward<Base>(other)), // base move is noexcept
		m_module(std::move(other.m_module)) // shared_ptr move is noexcept
	{}

	virtual ~WasmModuleInstance()
	{
		// wasm_runtime_deinstantiate(get());
	}

	/**
	 * @brief Copy is prohibited.
	 *
	 * @return WasmModuleInstance&
	 */
	WasmModuleInstance& operator=(const WasmModuleInstance&) = delete;

	/**
	 * @brief Move assignment operator.
	 *
	 * @param other Another WASM module instance object.
	 * @return This WASM module instance object after moving.
	 */
	WasmModuleInstance& operator=(WasmModuleInstance&& other)
	{
		Base::operator=(std::forward<Base>(other));
		m_module = std::move(other.m_module);
		return *this;
	}


	// template<typename _T>
	// typename Internal::WasmModMemIf<_T>::Single
	// Malloc()
	// {
	// 	using _MemType     = typename Internal::WasmModMemIf<_T>::Single;
	// 	using _MemBaseType = typename _MemType::Base;

	// 	return _MemBaseType::template Malloc<_MemType>(get(), sizeof(_T));
	// }

	// template<typename _T, typename... _Args>
	// typename Internal::WasmModMemIf<_T>::Single
	// New(_Args&&... args)
	// {
	// 	using _MemType     = typename Internal::WasmModMemIf<_T>::Single;

	// 	_MemType mem = Malloc<_T>();

	// 	new (mem.get()) _T(std::forward<_Args>(args)...);
	// 	mem.m_needDestr = true;

	// 	return mem;
	// }

	// template<typename _T>
	// typename Internal::WasmModMemIf<_T>::ArrayUnknownBound
	// Malloc(size_t n)
	// {
	// 	using _MemType     =
	// 		typename Internal::WasmModMemIf<_T>::ArrayUnknownBound;
	// 	using _MemBaseType = typename _MemType::Base;
	// 	using _ElemType    = typename _MemType::element_type;

	// 	size_t size = n * sizeof(_ElemType);
	// 	return _MemBaseType::template Malloc<_MemType>(get(), size);
	// }

	// template<typename _T>
	// typename Internal::WasmModMemIf<_T>::ArrayUnknownBound
	// New(size_t n)
	// {
	// 	using _MemType     =
	// 		typename Internal::WasmModMemIf<_T>::ArrayUnknownBound;
	// 	using _ElemType    = typename _MemType::element_type;
	// 	using _Up          = typename std::remove_extent<_T>::type;

	// 	static_assert(
	// 		std::is_same<_ElemType, _Up>::value,
	// 		"_Up must be the same as element_type");

	// 	_MemType mem = Malloc<_T>(n);

	// 	new (mem.get()) _Up[n]();
	// 	mem.m_numConstructedItems = n;

	// 	return mem;
	// }

private:

	std::shared_ptr<WasmModule> m_module;

}; // class WasmModuleInstance


} // namespace DecentWasmRuntime

