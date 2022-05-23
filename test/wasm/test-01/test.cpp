#include <cstddef>
#include <cstdint>

#include "DecentWasmApi.hpp"
#include "DecentWasmImpl.hpp"

extern "C" int decent_wasm_injected_main(
	const uint8_t* eIdSec, size_t eIdSecSize,
	const uint8_t* msgSec, size_t msgSecSize,
	size_t threshold
)
{
	decent_wasm_print("Hello World!\n");

	int tmp = decent_wasm_sum(1, 2);
	if (tmp == 3)
	{
		decent_wasm_print("Correct summation result\n");
	}
	else
	{
		decent_wasm_print("Wrong summation result\n");
	}

	for(size_t i = 0; i < 10; ++i)
	{
		decent_wasm_print("Loop");
	}

	return 0;
}
