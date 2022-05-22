#include <cstddef>
#include <cstdint>

#include <stdexcept>
#include <string>
#include <vector>

extern "C"
{
	int decent_wasm_sum(int a, int b);// { return 0; }
	void decent_wasm_print(const char * msg);// {}
}

extern "C" int decent_wasm_injected_main(
	const uint8_t* eIdSec, size_t eIdSecSize,
	const uint8_t* msgSec, size_t msgSecSize,
	size_t counterThreshold
)
{
	decent_wasm_print("Hello World!\n");

	std::vector<uint32_t> nums = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, };
	for(size_t i = 0; i < nums.size(); ++i)
	{
		auto numStr = std::to_string(nums[i]);
		auto outStr = "Loop: " + numStr + "\n";
		decent_wasm_print(outStr.c_str());
	}

	return 0;
}
