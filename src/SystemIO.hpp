// Copyright (c) 2024 Haofan Zheng
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstdint>
#include <cstring>

#include <stdexcept>
#include <string>

#ifdef DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED

#include <sgx_error.h>

extern "C" sgx_status_t ocall_print(const char* str);
extern "C" sgx_status_t ocall_decent_untrusted_timestamp_us(uint64_t* ret_val);

#else // !DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED

extern "C" void ocall_print(const char* str);
extern "C" uint64_t ocall_decent_untrusted_timestamp_us();

#endif // DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED


inline std::string InsertMsgHeader(
	const std::string& header,
	const std::string& msg
)
{
	bool lastIsSpace = true;
	std::string editedMsg;
	for (const auto& ch: msg)
	{
		if (ch == '\n' || ch == '\r')
		{
			lastIsSpace = true;
		}
		else
		{
			if (lastIsSpace)
			{
				editedMsg += header;
			}
			lastIsSpace = false;
		}
		editedMsg += ch;
	}
	return editedMsg;
}


#ifdef DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED
inline void PrintStr(const std::string& str)
{
	ocall_print(InsertMsgHeader("[Enclave] ", str).c_str());
}

inline uint64_t GetTimestampUs()
{
	uint64_t ret = 0;
	sgx_status_t sgxRet = ocall_decent_untrusted_timestamp_us(&ret);
	if (sgxRet != SGX_SUCCESS)
	{
		throw std::runtime_error("Failed to get timestamp from ocall");
	}
	return ret;
}
#endif // DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED


inline void PrintCStr(const char* str)
{
	PrintStr(str);
}

