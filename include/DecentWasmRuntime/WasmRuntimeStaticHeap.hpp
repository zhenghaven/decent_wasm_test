// Copyright (c) 2024 Haofan Zheng
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include "WasmRuntime.hpp"

#include <memory>

#include <wasm_export.h>

#include "Exception.hpp"
#include "Internal/make_unique.hpp"


extern "C" {

typedef void (*os_print_function_t)(const char *message);
extern void wasm_os_set_print_function(os_print_function_t pf);

extern void decent_wasm_reg_natives();

} // extern "C"


namespace DecentWasmRuntime
{


class WasmRuntimeStaticHeap :
	public WasmRuntime
{
public:

	using Base = WasmRuntime;

public:

	WasmRuntimeStaticHeap(
		os_print_function_t pf,
		uint32_t heapSize
	) :
		Base(),
		m_pf(pf),
		m_heapSize(heapSize),
		m_heap(Internal::make_unique<uint8_t[]>(heapSize))
	{
		RuntimeInitArgs init_args;
		std::memset(&init_args, 0, sizeof(RuntimeInitArgs));

		init_args.mem_alloc_type = Alloc_With_Pool;
		init_args.mem_alloc_option.pool.heap_buf = m_heap.get();
		init_args.mem_alloc_option.pool.heap_size = heapSize;

		/* initialize runtime environment */
		if (!wasm_runtime_full_init(&init_args))
		{
			throw Exception("Init runtime environment failed");
		}
		wasm_os_set_print_function(pf);
		decent_wasm_reg_natives();
	}

	WasmRuntimeStaticHeap(const WasmRuntimeStaticHeap&) = delete;

	WasmRuntimeStaticHeap(WasmRuntimeStaticHeap&& other) = delete;

	virtual ~WasmRuntimeStaticHeap() noexcept
	{
		//wasm_runtime_memory_destroy();
		wasm_runtime_destroy();
		m_heap.reset();
	}

	WasmRuntimeStaticHeap& operator=(const WasmRuntimeStaticHeap&) = delete;

	WasmRuntimeStaticHeap& operator=(WasmRuntimeStaticHeap&&) = delete;

private:

	os_print_function_t m_pf;
	uint32_t m_heapSize;
	std::unique_ptr<uint8_t[]> m_heap;

}; // class WasmRuntimeStaticHeap


} // namespace DecentWasmRuntime

