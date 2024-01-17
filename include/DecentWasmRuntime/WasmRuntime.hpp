// Copyright (c) 2024 Haofan Zheng
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


typedef void (*os_print_function_t)(const char *message);


namespace DecentWasmRuntime
{


class WasmRuntime
{
public:

	WasmRuntime(os_print_function_t printFunc) :
		m_printFunc(printFunc)
	{}

	WasmRuntime(const WasmRuntime&) = delete;

	WasmRuntime(WasmRuntime&&) = delete;

	virtual ~WasmRuntime() noexcept
	{}

	WasmRuntime& operator=(const WasmRuntime&) = delete;

	WasmRuntime& operator=(WasmRuntime&&) = delete;

	os_print_function_t GetPrintFunc() const noexcept
	{
		return m_printFunc;
	}

private:

	os_print_function_t m_printFunc;
}; // class WasmRuntime


} // namespace DecentWasmRuntime

