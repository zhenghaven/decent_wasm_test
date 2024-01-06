// Copyright (c) 2024 Haofan Zheng
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <stdexcept>


namespace DecentWasmRuntime
{


class Exception : public std::runtime_error
{
public:
	Exception(const char* msg) : std::runtime_error(msg) {}
	Exception(const std::string& msg) : std::runtime_error(msg) {}

	virtual ~Exception() noexcept {}
}; // class Exception


} // namespace DecentWasmRuntime

