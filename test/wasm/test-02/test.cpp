#include <cstddef>
#include <cstdint>

#include <stdexcept>
#include <string>

#include "DecentWasmApi.hpp"
#include "DecentWasmImpl.hpp"

extern "C" int32_t decent_wasm_injected_main(
	const uint8_t* eIdSec, uint32_t eIdSecSize,
	const uint8_t* msgSec, uint32_t msgSecSize,
	uint64_t threshold
)
{
	decent_wasm_print("Hello World!\n");

	try
	{
		throw std::runtime_error("test try/catch");
	}
	catch(const std::exception& e)
	{
		decent_wasm_print("Exception caught: ");
		decent_wasm_print(e.what());
		decent_wasm_print("\n");
	}

	throw std::runtime_error("test throw");

	return 0;
}
