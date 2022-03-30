cmake_minimum_required(VERSION 3.18)

project(decent-wasm-test LANGUAGES C CXX VERSION 0.0.1)

include(FetchContent)
include(cmake/IntelSgxAddTarget.cmake)

# Compile options

if(MSVC)
	set(COMMON_OPTIONS /W3 /wd4996 /we4239 /we4002 /we4700 /we4305 /EHsc /MP
		/GR /Zc:__cplusplus)
	set(DEBUG_OPTIONS /MTd /Od /Zi /DDEBUG)
	set(RELEASE_OPTIONS /MT /Ox /Oi /Ob2 /fp:fast)# /DNDEBUG
#	set(CMAKE_EXE_LINKER_FLAGS_DEBUG ${CMAKE_EXE_LINKER_FLAGS_DEBUG} /DEBUG)
else()
	set(COMMON_OPTIONS -pthread)
	set(DEBUG_OPTIONS -O0 -g -DDEBUG)
	set(RELEASE_OPTIONS -O2) #-DNDEBUG defined by default
endif()

set(DEBUG_OPTIONS ${COMMON_OPTIONS} ${DEBUG_OPTIONS})
set(RELEASE_OPTIONS ${COMMON_OPTIONS} ${RELEASE_OPTIONS})

#Removed Basic Runtime Checks in MSVC
if(MSVC)
	STRING (REGEX REPLACE "/RTC(su|[1su])" ""
		CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
	STRING (REGEX REPLACE "/RTC(su|[1su])" ""
		CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
	STRING (REGEX REPLACE "/RTC(su|[1su])" ""
		CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
	STRING (REGEX REPLACE "/RTC(su|[1su])" ""
		CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
endif()

# Setup WASM
FetchContent_Declare(
	git_wasm_micro_runtime
	GIT_REPOSITORY https://github.com/zhenghaven/wasm-micro-runtime.git
	GIT_TAG        origin/decent-sgx
	SOURCE_SUBDIR  product-mini/platforms/decent-sgx
)
FetchContent_MakeAvailable(git_wasm_micro_runtime)

# Setup WABT
FetchContent_Declare(
	git_wabt_decent_sgx
	GIT_REPOSITORY https://github.com/zhenghaven/wabt.git
	GIT_TAG        origin/decent-enclave
	SOURCE_SUBDIR  decent/sgx
)
FetchContent_MakeAvailable(git_wabt_decent_sgx)

# Add DebugSimulation to CMake configuration types
set(CMAKE_CONFIGURATION_TYPES Release Debug DebugSimulation)
set_property(GLOBAL PROPERTY DEBUG_CONFIGURATIONS Debug DebugSimulation)

set(CMAKE_CXX_FLAGS_DEBUGSIMULATION           ${CMAKE_CXX_FLAGS_DEBUG})
set(CMAKE_C_FLAGS_DEBUGSIMULATION             ${CMAKE_C_FLAGS_DEBUG})
set(CMAKE_EXE_LINKER_FLAGS_DEBUGSIMULATION    ${CMAKE_EXE_LINKER_FLAGS_DEBUG})
set(CMAKE_SHARED_LINKER_FLAGS_DEBUGSIMULATION ${CMAKE_SHARED_LINKER_FLAGS_DEBUG})
set(CMAKE_STATIC_LINKER_FLAGS_DEBUGSIMULATION ${CMAKE_STATIC_LINKER_FLAGS_DEBUG})

# Add source dirs
add_subdirectory(src)
