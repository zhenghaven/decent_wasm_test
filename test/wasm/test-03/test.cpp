#include <cstddef>
#include <cstdint>

#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "DecentWasmApi.hpp"
#include "DecentWasmImpl.hpp"

template<typename _It>
std::string Array2Str(_It begin, _It end)
{
	using _OriValType = typename std::iterator_traits<_It>::value_type;

	using _ValType = typename std::conditional<
		std::is_integral<_OriValType>::value &&
		(std::numeric_limits<_OriValType>::digits <= 8),
		int,
		_OriValType>::type;

	std::string res;

	if (begin != end)
	{
		_ValType val = *begin;
		res += std::to_string(val);
		++begin;
	}

	for (; begin != end; ++begin)
	{
		_ValType val = *begin;
		res += ", " + std::to_string(val);
	}

	return res;
}

extern "C" int32_t decent_wasm_injected_main(
	const uint8_t* eIdSec, uint32_t eIdSecSize,
	const uint8_t* msgSec, uint32_t msgSecSize,
	uint64_t threshold
)
{
	size_t printLen;
	std::string outStr;

	decent_wasm_print("Hello World!\n");

	outStr = "ID Sec Size: " + std::to_string(eIdSecSize) + "\n";
	decent_wasm_print(outStr.c_str());
	outStr = "Msg Sec Size: " + std::to_string(msgSecSize) + "\n";
	decent_wasm_print(outStr.c_str());

	printLen = eIdSecSize <= 10 ? eIdSecSize : 10;
	outStr =
		"ID Sec: " +
		Array2Str(eIdSec, eIdSec + printLen) +
		"\n";
	decent_wasm_print(outStr.c_str());

	printLen = msgSecSize <= 10 ? msgSecSize : 10;
	outStr =
		"Msg Sec: " +
		Array2Str(msgSec, msgSec + printLen) +
		"\n";
	decent_wasm_print(outStr.c_str());

	std::vector<uint32_t> nums = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, };
	outStr =
		"Testing loop: " +
		Array2Str(nums.begin(), nums.end()) +
		"\n";
	decent_wasm_print(outStr.c_str());

	return 0;
}
