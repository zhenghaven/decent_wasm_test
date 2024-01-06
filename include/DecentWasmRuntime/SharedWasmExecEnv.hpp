// Copyright (c) 2024 Haofan Zheng
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include "SharedObject.hpp"

#include <memory>

#include "Internal/make_unique.hpp"
#include "WasmExecEnv.hpp"


namespace DecentWasmRuntime
{


class SharedWasmExecEnv :
	public SharedObject<WasmExecEnv>
{
public: // static members

	using Base = SharedObject<WasmExecEnv>;

public:

	using Base::Base;


}; // class SharedWasmExecEnv


} // namespace DecentWasmRuntime



