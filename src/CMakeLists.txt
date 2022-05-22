
add_sgx_target(decent_wasm_test
	UNTRUSTED_SOURCE   ${CMAKE_CURRENT_LIST_DIR}/Main.cpp
	UNTRUSTED_DEF      ""
	UNTRUSTED_COMP_OPT $<$<CONFIG:Debug>:${DEBUG_OPTIONS}>
						$<$<CONFIG:DebugSimulation>:${DEBUG_OPTIONS}>
						$<$<CONFIG:Release>:${RELEASE_OPTIONS}>
	UNTRUSTED_LINK_OPT ""
	UNTRUSTED_LINK_LIB ""
	TRUSTED_SOURCE      ${CMAKE_CURRENT_LIST_DIR}/Enclave.cpp
						${CMAKE_CURRENT_LIST_DIR}/decent_wasm_natives.c
	TRUSTED_DEF        ""
	TRUSTED_COMP_OPT   $<$<CONFIG:Debug>:${DEBUG_OPTIONS}>
						$<$<CONFIG:DebugSimulation>:${DEBUG_OPTIONS}>
						$<$<CONFIG:Release>:${RELEASE_OPTIONS}>
	TRUSTED_LINK_OPT   ""
	TRUSTED_LINK_LIB   vmlib_decent_sgx DecentWasmWat_core DecentWasmWat
	EDL_PATH           ${CMAKE_CURRENT_LIST_DIR}/Enclave.edl
	EDL_INCLUDE        ""
	EDL_OUTPUT_DIR     ${CMAKE_CURRENT_LIST_DIR}
	SIGN_CONFIG        ${CMAKE_CURRENT_LIST_DIR}/Enclave.config.xml
	SIGN_KEY           ${CMAKE_CURRENT_LIST_DIR}/Enclave_private.pem
)
