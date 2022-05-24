# decent_wasm_test
testing iwasm in Decent CMake build environment

## How to run the program

```shell
# Create build directory
mkdir build && cd build

# Initialize CMake project and build
source /opt/intel/sgxsdk/environment
# CMAKE_BUILD_TYPE could be Debug, DebugSimulation, Release
cmake -DCMAKE_BUILD_TYPE=DebugSimulation ../
make decent_wasm_test

# Run test
cd src
./decent_wasm_test ../../test/wasm/test-03/test.wasm

```
