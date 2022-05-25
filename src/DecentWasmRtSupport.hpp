// Copyright (c) 2022 Haofan Zheng
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <cstring>

#include <limits>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include <wasm_export.h>

namespace Decent
{

namespace WasmRt
{

template<typename _T, typename _Deleter>
struct WamrUniquePtr
{

	typedef _T                                             element_type;
	typedef _Deleter                                       deleter_type;
	typedef typename std::add_pointer<element_type>::type  pointer;
	typedef typename std::add_const<deleter_type>::type    const_deleter_type;

	typedef typename std::add_lvalue_reference<deleter_type>::type
						deleter_reference_type;
	typedef typename std::add_lvalue_reference<const_deleter_type>::type
						const_deleter_reference_type;

	WamrUniquePtr(pointer ptr) noexcept : m_ptr(ptr), m_del() {}

	virtual ~WamrUniquePtr() { reset(); }

	pointer get() const noexcept { return m_ptr; }

	void reset(pointer ptr = nullptr) noexcept;

	deleter_reference_type get_deleter() noexcept { return m_del; }

	const_deleter_reference_type get_deleter() const noexcept { return m_del; }

	pointer m_ptr;
	deleter_type m_del;

}; // struct WamrUniquePtr


template<typename _PtrType>
class WasmModMemCore
{

public: // static members:

	using Self       = WasmModMemCore<_PtrType>;

	typedef _PtrType  pointer;
	typedef uint32_t  rel_pointer;

	using _SizeType  = rel_pointer;

	template<typename _RetType>
	static _RetType Malloc(wasm_module_inst_t modInst, size_t size)
	{
		if (size > std::numeric_limits<_SizeType>::max())
		{
			throw std::bad_alloc();
		}

		_SizeType sizeInternal = static_cast<_SizeType>(size);
		void* ptr = nullptr;
		rel_pointer relPtr =
			wasm_runtime_module_malloc(modInst, sizeInternal, &ptr);
		if (!relPtr)
		{
			throw std::bad_alloc();
		}

		return _RetType(
			modInst, relPtr, reinterpret_cast<pointer>(ptr), sizeInternal);
	}

public:

	WasmModMemCore(
		wasm_module_inst_t modInst,
		rel_pointer        relPtr,
		pointer            ptr,
		_SizeType          size) noexcept :
		m_modInst(modInst),
		m_relPtr(relPtr),
		m_ptr(ptr),
		m_size(size)
	{}

	virtual ~WasmModMemCore()
	{
		wasm_runtime_module_free(m_modInst, m_relPtr);
		m_relPtr = 0;
		m_ptr = nullptr;
	}

	pointer get() const noexcept { return m_ptr; }

	rel_pointer GetRelPtr() const noexcept { return m_relPtr; }

	_SizeType size() const noexcept { return m_size; }

private:

	wasm_module_inst_t  m_modInst;
	rel_pointer         m_relPtr;
	pointer             m_ptr;
	_SizeType           m_size;

}; // struct WasmModMemCore


template<typename _T>
class WasmModMem :
	public WasmModMemCore<typename std::add_pointer<_T>::type>
{
public: // static members:

	using Base       =
		WasmModMemCore<typename std::add_pointer<_T>::type>;
	using Self       = WasmModMem<_T>;

	typedef _T                          element_type;
	typedef typename Base::pointer      pointer;
	typedef typename Base::rel_pointer  rel_pointer;

	using _SizeType = typename Base::_SizeType;

public:

	WasmModMem(
		wasm_module_inst_t modInst,
		rel_pointer        relPtr,
		pointer            ptr,
		_SizeType          size) noexcept :
		Base::WasmModMemCore(modInst, relPtr, ptr, size),
		m_needDestr(false)
	{}

	virtual ~WasmModMem()
	{
		if (m_needDestr)
		{
			Base::get()->~element_type();
			m_needDestr = false;
		}
	}

	bool m_needDestr;

}; // struct WasmModMem


template<typename _T>
class WasmModMem<_T[]> :
	public WasmModMemCore<typename std::add_pointer<_T>::type>
{
public: // static members:

	using Base       =
		WasmModMemCore<typename std::add_pointer<_T>::type>;
	using Self       = WasmModMem<_T>;

	typedef _T                          element_type;
	typedef typename Base::pointer      pointer;
	typedef typename Base::rel_pointer  rel_pointer;

	typedef typename std::add_lvalue_reference<element_type>::type
						reference_type;

	using _SizeType = typename Base::_SizeType;

public:

	WasmModMem(
		wasm_module_inst_t modInst,
		rel_pointer        relPtr,
		pointer            ptr,
		_SizeType          size) noexcept :
		Base::WasmModMemCore(modInst, relPtr, ptr, size),
		m_numConstructedItems(0)
	{}

	virtual ~WasmModMem()
	{
		while (m_numConstructedItems > 0)
		{
			Base::get()[--m_numConstructedItems].~element_type();
		}
	}

	reference_type operator[](size_t idx) const { return Base::get()[idx]; }

	size_t m_numConstructedItems;

}; // struct WasmModMem


namespace Internal
{

template<class _T>
struct WasmModMemIf
{
	typedef WasmModMem<_T> Single;
};

template<class _T>
struct WasmModMemIf<_T[]>
{
	typedef WasmModMem<_T[]> ArrayUnknownBound;
};

template<class _T, size_t _N>
struct WasmModMemIf<_T[_N]>
{
	typedef void ArrayKnownBound;
};

} // namespace Internal


struct WasmModule;
struct WasmModuleInstance;
struct WasmExecEnv;


struct WasmModuleUnload
{
	void operator()(wasm_module_t ptr) noexcept;
}; // struct WasmModuleUnload

struct WasmModule :
	public WamrUniquePtr<
		typename std::remove_pointer<wasm_module_t>::type,
		WasmModuleUnload>
{
	using Base = WamrUniquePtr<
		typename std::remove_pointer<wasm_module_t>::type,
		WasmModuleUnload>;

	using Base::WamrUniquePtr;

	static WasmModule Load(const std::vector<uint8_t>& wasm);

	WasmModuleInstance Instantiate(uint32_t stackSize, uint32_t heapSize);

}; // struct WasmModule


struct WasmModuleDeinstantiate
{
	void operator()(wasm_module_inst_t ptr) noexcept;
}; // struct WasmModuleDeinstantiate

struct WasmModuleInstance :
	public WamrUniquePtr<
		typename std::remove_pointer<wasm_module_inst_t>::type,
		WasmModuleDeinstantiate>
{
	using Base = WamrUniquePtr<
		typename std::remove_pointer<wasm_module_inst_t>::type,
		WasmModuleDeinstantiate>;

	using Base::WamrUniquePtr;

	WasmExecEnv CreateExecEnv(uint32_t stackSize);

	template<typename _T>
	typename Internal::WasmModMemIf<_T>::Single
	Malloc()
	{
		using _MemType     = typename Internal::WasmModMemIf<_T>::Single;
		using _MemBaseType = typename _MemType::Base;

		return _MemBaseType::template Malloc<_MemType>(get(), sizeof(_T));
	}

	template<typename _T, typename... _Args>
	typename Internal::WasmModMemIf<_T>::Single
	New(_Args&&... args)
	{
		using _MemType     = typename Internal::WasmModMemIf<_T>::Single;

		_MemType mem = Malloc<_T>();

		new (mem.get()) _T(std::forward<_Args>(args)...);
		mem.m_needDestr = true;

		return mem;
	}

	template<typename _T>
	typename Internal::WasmModMemIf<_T>::ArrayUnknownBound
	Malloc(size_t n)
	{
		using _MemType     =
			typename Internal::WasmModMemIf<_T>::ArrayUnknownBound;
		using _MemBaseType = typename _MemType::Base;
		using _ElemType    = typename _MemType::element_type;

		size_t size = n * sizeof(_ElemType);
		return _MemBaseType::template Malloc<_MemType>(get(), size);
	}

	template<typename _T>
	typename Internal::WasmModMemIf<_T>::ArrayUnknownBound
	New(size_t n)
	{
		using _MemType     =
			typename Internal::WasmModMemIf<_T>::ArrayUnknownBound;
		using _ElemType    = typename _MemType::element_type;
		using _Up          = typename std::remove_extent<_T>::type;

		static_assert(
			std::is_same<_ElemType, _Up>::value,
			"_Up must be the same as element_type");

		_MemType mem = Malloc<_T>(n);

		new (mem.get()) _Up[n]();
		mem.m_numConstructedItems = n;

		return mem;
	}

}; // struct WasmModuleInstance


struct WasmExecEnvDestroy
{
	void operator()(wasm_exec_env_t ptr) noexcept;
}; // struct WasmExecEnvDestroy

struct WasmExecEnv :
	public WamrUniquePtr<
		typename std::remove_pointer<wasm_exec_env_t>::type,
		WasmExecEnvDestroy>
{
	using Base = WamrUniquePtr<
		typename std::remove_pointer<wasm_exec_env_t>::type,
		WasmExecEnvDestroy>;

	using Base::WamrUniquePtr;

	template<typename _RetTuple, typename... _Args>
	inline _RetTuple ExecFunc(const std::string& funcName,
		_Args&&... args);

}; // struct WasmExecEnv

} // namespace WasmRt

} // namespace Decent


//============================================================
// Implementation
//============================================================


namespace Decent
{

namespace WasmRt
{


template<typename _T, typename _Deleter>
inline void WamrUniquePtr<_T, _Deleter>::reset(pointer ptr) noexcept
{
	pointer tmp = m_ptr;
	m_ptr = ptr;
	if (tmp != nullptr)
	{
		get_deleter()(tmp);
	}
}





inline void WasmModuleUnload::operator()(wasm_module_t ptr) noexcept
{
	wasm_runtime_unload(ptr);
}

inline WasmModule WasmModule::Load(
	const std::vector<uint8_t>& wasm)
{
	char errorBuf[128];

	wasm_module_t ptr = wasm_runtime_load(
		wasm.data(), wasm.size(),
		errorBuf, sizeof(errorBuf)
	);

	if (ptr == nullptr)
	{
		throw std::runtime_error(errorBuf);
	}

	return WasmModule(ptr);
}

inline WasmModuleInstance WasmModule::Instantiate(
	uint32_t stackSize, uint32_t heapSize)
{
	char errorBuf[128];

	wasm_module_inst_t ptr = wasm_runtime_instantiate(
		get(), stackSize, heapSize,
		errorBuf, sizeof(errorBuf)
	);

	if (ptr == nullptr)
	{
		throw std::runtime_error(errorBuf);
	}

	return WasmModuleInstance(ptr);
}





inline void WasmModuleDeinstantiate::operator()(wasm_module_inst_t ptr) noexcept
{
	wasm_runtime_deinstantiate(ptr);
}

inline WasmExecEnv WasmModuleInstance::CreateExecEnv(uint32_t stackSize)
{
	wasm_exec_env_t ptr = wasm_runtime_create_exec_env(get(), stackSize);

	if (ptr == nullptr)
	{
		throw std::runtime_error("Failed to create execution environment");
	}

	return WasmExecEnv(ptr);
}





inline void WasmExecEnvDestroy::operator()(wasm_exec_env_t ptr) noexcept
{
	wasm_runtime_destroy_exec_env(ptr);
}





// template<typename _T>
// inline void* RuntimeMem<_T>::Malloc(
// 	uint64_t size, wasm_module_inst_t moduleInst)
// {
// 	if (size >= std::numeric_limits<uint32_t>::max())
// 	{
// 		if (moduleInst != nullptr)
// 		{
// 			wasm_runtime_set_exception(moduleInst,
// 				"memory allocation size exceeds maximum limit");
// 		}
// 		throw std::bad_alloc();
// 	}

// 	const uint32_t size32 = static_cast<uint32_t>(size);
// 	void* mem = wasm_runtime_malloc(size32);

// 	if (mem == nullptr)
// 	{
// 		throw std::bad_alloc();
// 	}

// 	std::memset(mem, 0, size32);
// 	return mem;
// }

// template<typename _T>
// inline RuntimeMem<_T> RuntimeMem<_T>::NewArray(
// 	size_t n, wasm_module_inst_t moduleInst)
// {
// 	size_t spaceSize = n * sizeof(_T);

// 	return Self(reinterpret_cast<_T*>(
// 			Self::Malloc(spaceSize, moduleInst)
// 	));
// }

// template<typename _T>
// inline RuntimeMem<_T>::~RuntimeMem()
// {
// 	wasm_runtime_free(m_ptr);
// 	m_ptr = nullptr;
// }





namespace Internal
{

template<typename _T>
struct ToWasmVal;

template<>
struct ToWasmVal<uint64_t>
{
	void operator()(wasm_val_t& wasmVal, uint64_t val)
	{
		wasmVal.kind = wasm_valkind_enum::WASM_I64;
		wasmVal.of.i64 = val;
	}
}; // struct ToWasmVal<uint64_t>

template<>
struct ToWasmVal<uint32_t>
{
	void operator()(wasm_val_t& wasmVal, uint32_t val)
	{
		wasmVal.kind = wasm_valkind_enum::WASM_I32;
		wasmVal.of.i32 = val;
	}
}; // struct ToWasmVal<uint32_t>

template<typename _PtrType>
struct ToWasmVal<WasmModMemCore<_PtrType> >
{
	void operator()(wasm_val_t& wasmVal, const WasmModMemCore<_PtrType>& val)
	{
		wasmVal.kind = wasm_valkind_enum::WASM_I32;
		wasmVal.of.i32 = val.GetRelPtr();
	}
}; // struct ToWasmVal<WasmModMemCore<_PtrType> >

template<typename _T>
struct ToWasmVal<WasmModMem<_T> > : ToWasmVal<typename WasmModMem<_T>::Base>
{}; // struct ToWasmVal<WasmModMem<_T> >

template<size_t _TotalSize>
inline void ParamPack2WasmValList(wasm_val_t (&wasmArg)[_TotalSize + 1])
{
	return;
}

template<size_t _TotalSize, typename _Arg1, typename... _Args>
inline void ParamPack2WasmValList(wasm_val_t (&wasmArg)[_TotalSize + 1],
	_Arg1&& arg1, _Args&&... args)
{
	static constexpr size_t left = 1 + sizeof...(_Args);
	static_assert(_TotalSize >= left,
		"The total size of args must be greater than or equal to what left");
	static constexpr size_t idx = _TotalSize - left;
	static_assert(idx < _TotalSize,
		"The index must be within the range of total size of args");

	using ToWasmValType = ToWasmVal<
		typename std::remove_const<
			typename std::remove_reference<_Arg1>::type
		>::type
	>;

	ToWasmValType()(wasmArg[idx], std::forward<_Arg1>(arg1));

	ParamPack2WasmValList<_TotalSize>(wasmArg, std::forward<_Args>(args)...);
}

template<typename _TupleType, size_t _ItemLeft>
struct WasmValList2Tuple;

template<typename _TupleType>
struct WasmValList2Tuple<_TupleType, 0>
{
	void operator()(
		_TupleType& tp,
		const wasm_val_t (&wasmArg)[std::tuple_size<_TupleType>::value + 1])
	{
		return;
	}
}; // struct WasmValList2Tuple<_TupleType, 0>

template<typename _TupleType, size_t _ItemLeft>
struct WasmValList2Tuple
{
	void operator()(
	_TupleType& tp,
	const wasm_val_t (&wasmArg)[std::tuple_size<_TupleType>::value + 1])
	{
		static constexpr size_t totalSize = std::tuple_size<_TupleType>::value;
		static_assert(_ItemLeft <= totalSize,
			"The num of items left must be within the range of the total size");
		static constexpr size_t _Idx = totalSize - _ItemLeft;

		const wasm_val_t& wasmVal = wasmArg[_Idx];
		switch(wasmVal.kind)
		{
		case wasm_valkind_enum::WASM_I32:
			std::get<_Idx>(tp) = wasmVal.of.i32;
			break;

		case wasm_valkind_enum::WASM_I64:
			std::get<_Idx>(tp) = wasmVal.of.i64;
			break;

		case wasm_valkind_enum::WASM_F32:
			std::get<_Idx>(tp) = wasmVal.of.f32;
			break;

		case wasm_valkind_enum::WASM_F64:
			std::get<_Idx>(tp) = wasmVal.of.f64;
			break;

		default:
			throw std::runtime_error(
				"The given type of WASM value is not supported");
		}

		WasmValList2Tuple<_TupleType, _ItemLeft - 1>()(tp, wasmArg);
	}
}; // struct WasmValList2Tuple

} // namespace Internal

template<typename _RetTuple, typename... _Args>
inline _RetTuple WasmExecEnv::ExecFunc(
	const std::string& funcName,
	_Args&&... args)
{
	static constexpr uint32_t numRes = std::tuple_size<_RetTuple>::value;
	static constexpr uint32_t numArg = sizeof...(_Args);

	_RetTuple retVals;
	wasm_val_t wasmRes[numRes + 1];
	wasm_val_t wasmArg[numArg + 1];

	Internal::ParamPack2WasmValList<numArg>(
		wasmArg, std::forward<_Args>(args)...);

	wasm_module_inst_t   moduleInst = wasm_runtime_get_module_inst(get());
	wasm_function_inst_t targetFunc =
		wasm_runtime_lookup_function(moduleInst, funcName.c_str(), nullptr);
	if (targetFunc == nullptr)
	{
		throw std::invalid_argument(
			"Could not find the function named " +
			funcName +
			" in the given WASM module");
	}
	// may be needed for future work on multi-mod:
	// wasm_module_inst_t   targetInst = moduleInst;

	bool execRes = wasm_runtime_call_wasm_a(
		get(), targetFunc,
		numRes, wasmRes,
		numArg, wasmArg);

	if (!execRes)
	{
		throw std::runtime_error(wasm_runtime_get_exception(moduleInst));
	}

	Internal::WasmValList2Tuple<_RetTuple, numRes>()(retVals, wasmRes);

	return retVals;
}

} // namespace WasmRt

} // namespace Decent

