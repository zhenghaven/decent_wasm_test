cmake_minimum_required(VERSION 3.18)

project(decent-wasm-test LANGUAGES C CXX)

include(FetchContent)
include(cmake/IntelSgxAddTarget.cmake)

# Setup WASM
FetchContent_Declare(
	git_wasm_micro_runtime
	GIT_REPOSITORY https://github.com/zhenghaven/wasm-micro-runtime.git
	GIT_TAG        origin/decent-sgx
	SOURCE_SUBDIR  product-mini/platforms/decent-sgx
)
FetchContent_MakeAvailable(git_wasm_micro_runtime)

# CMake configuration types
set(CMAKE_CONFIGURATION_TYPES Release Debug DebugSimulation)
set_property(GLOBAL PROPERTY DEBUG_CONFIGURATIONS Debug DebugSimulation)

# Add source dirs
add_subdirectory(src)
