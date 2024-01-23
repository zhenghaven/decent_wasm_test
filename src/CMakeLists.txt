# Copyright (c) 2023 Haofan Zheng
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.


include(DecentEnclaveIntelSgx)


decent_enclave_print_config_sgx()


decent_enclave_add_target_sgx(decent_wasm_test
	UNTRUSTED_SOURCE
		${CMAKE_CURRENT_LIST_DIR}/Main.cpp
		${CMAKE_CURRENT_LIST_DIR}/decent_wasm_natives.c
		${CMAKE_CURRENT_LIST_DIR}/DecentWasmNatives.cpp
	UNTRUSTED_DEF
		DECENTENCLAVE_DEV_LEVEL_0
	UNTRUSTED_INCL_DIR
		""
	UNTRUSTED_COMP_OPT
		$<$<CONFIG:Debug>:${DEBUG_OPTIONS}>
		$<$<CONFIG:DebugSimulation>:${DEBUG_OPTIONS}>
		$<$<CONFIG:Release>:${RELEASE_OPTIONS}>
	UNTRUSTED_LINK_OPT ""
	UNTRUSTED_LINK_LIB
		DecentWasmRuntime
		iwasm_static
	TRUSTED_SOURCE
		${CMAKE_CURRENT_LIST_DIR}/Enclave.cpp
		${CMAKE_CURRENT_LIST_DIR}/decent_wasm_natives.c
		${CMAKE_CURRENT_LIST_DIR}/DecentWasmNatives.cpp
	TRUSTED_DEF
		DECENTENCLAVE_DEV_LEVEL_0
	TRUSTED_INCL_DIR
		""
	TRUSTED_COMP_OPT
		$<$<CONFIG:Debug>:${DEBUG_OPTIONS}>
		$<$<CONFIG:DebugSimulation>:${DEBUG_OPTIONS}>
		$<$<CONFIG:Release>:${RELEASE_OPTIONS}>
	TRUSTED_LINK_OPT   ""
	TRUSTED_LINK_LIB
		vmlib_decent_sgx
		DecentWasmRuntime
		DecentWasmWat_core
		DecentWasmWat
	EDL_PATH
		${CMAKE_CURRENT_LIST_DIR}/Enclave.edl
	EDL_INCLUDE
		""
	EDL_OUTPUT_DIR
		${CMAKE_CURRENT_LIST_DIR}
	SIGN_CONFIG
		${CMAKE_CURRENT_LIST_DIR}/Enclave.config.xml
	SIGN_KEY
		${CMAKE_CURRENT_LIST_DIR}/Enclave_private.pem
)
