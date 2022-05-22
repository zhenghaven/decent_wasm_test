#include <cstddef>
#include <cstdint>

#include <stdexcept>
#include <string>

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
