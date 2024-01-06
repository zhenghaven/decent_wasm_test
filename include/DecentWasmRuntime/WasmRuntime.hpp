// Copyright (c) 2024 Haofan Zheng
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


namespace DecentWasmRuntime
{


class WasmRuntime
{
public:

	WasmRuntime()
	{}

	WasmRuntime(const WasmRuntime&) = delete;

	WasmRuntime(WasmRuntime&&) = delete;

	virtual ~WasmRuntime() noexcept
	{}

	WasmRuntime& operator=(const WasmRuntime&) = delete;

	WasmRuntime& operator=(WasmRuntime&&) = delete;

}; // class WasmRuntime


} // namespace DecentWasmRuntime

