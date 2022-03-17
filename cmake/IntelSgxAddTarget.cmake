cmake_minimum_required(VERSION 3.13)

include(${CMAKE_CURRENT_LIST_DIR}/IntelSgxSdkLibs.cmake)

if(MSVC)
	set(BINARY_SUB_DIR "$<CONFIG>")
	set(WHOLE_ARCHIVE_FLAG_BEGIN "")
	set(WHOLE_ARCHIVE_FLAG_END "")
	set(GROUP_FLAG_BEGIN "")
	set(GROUP_FLAG_END "")
else()
	set(BINARY_SUB_DIR "")
	set(WHOLE_ARCHIVE_FLAG_BEGIN -Wl,--whole-archive)
	set(WHOLE_ARCHIVE_FLAG_END -Wl,--no-whole-archive)
	set(GROUP_FLAG_BEGIN -Wl,--start-group)
	set(GROUP_FLAG_END -Wl,--end-group)
endif()

# FUNCTION add_sgx_target(<TARGET_NAME>
# 		UNTRUSTED_SOURCE   <src1>;<src2>;...
# 		UNTRUSTED_DEF      <def1>;<def2>;...
# 		UNTRUSTED_COMP_OPT <opt1>;<opt2>;...
# 		UNTRUSTED_LINK_OPT <opt1>;<opt2>;...
# 		UNTRUSTED_LINK_LIB <lib1>;<lib2>;...
# 		TRUSTED_SOURCE     <src1>;<src2>;...
# 		TRUSTED_DEF        <def1>;<def2>;...
# 		TRUSTED_COMP_OPT   <opt1>;<opt2>;...
# 		TRUSTED_LINK_OPT   <opt1>;<opt2>;...
# 		TRUSTED_LINK_LIB   <lib1>;<lib2>;...
# 		EDL_PATH           <path>
# 		EDL_INCLUDE        <path1>;<path2>;...
# 		EDL_OUTPUT_DIR     <dir>
# 		SIGN_CONFIG        <path>
# 		SIGN_KEY           <path>
# )
FUNCTION(add_sgx_target)
	if (${ARGC} LESS 31)
		message(FATAL "Not enough arguments passed to add_sgx_target function")
	endif()

	set(_SGX_TARGET_TARGET_NAME "${ARGV0}")
	set(options "")
	set(oneValueArgs EDL_PATH EDL_OUTPUT_DIR SIGN_CONFIG SIGN_KEY)
	set(multiValueArgs
		UNTRUSTED_SOURCE UNTRUSTED_DEF
		UNTRUSTED_COMP_OPT UNTRUSTED_LINK_OPT UNTRUSTED_LINK_LIB
		TRUSTED_SOURCE   TRUSTED_DEF
		TRUSTED_COMP_OPT   TRUSTED_LINK_OPT   TRUSTED_LINK_LIB
		EDL_INCLUDE)

	cmake_parse_arguments(PARSE_ARGV 1 _SGX_TARGET "${options}" "${oneValueArgs}"
		"${multiValueArgs}")

	message(STATUS "==================== INTEL SGX TARGET ====================")
	# message(STATUS "ARGN = ${ARGN}")
	message(STATUS "TARGET_NAME        = ${_SGX_TARGET_TARGET_NAME}")
	message(STATUS "UNTRUSTED_SOURCE   = ${_SGX_TARGET_UNTRUSTED_SOURCE}")
	message(STATUS "UNTRUSTED_DEF      = ${_SGX_TARGET_UNTRUSTED_DEF}")
	message(STATUS "UNTRUSTED_COMP_OPT = ${_SGX_TARGET_UNTRUSTED_COMP_OPT}")
	message(STATUS "UNTRUSTED_LINK_OPT = ${_SGX_TARGET_UNTRUSTED_LINK_OPT}")
	message(STATUS "UNTRUSTED_LINK_LIB = ${_SGX_TARGET_UNTRUSTED_LINK_LIB}")
	message(STATUS "TRUSTED_SOURCE     = ${_SGX_TARGET_TRUSTED_SOURCE}")
	message(STATUS "TRUSTED_DEF        = ${_SGX_TARGET_TRUSTED_DEF}")
	message(STATUS "TRUSTED_COMP_OPT   = ${_SGX_TARGET_TRUSTED_COMP_OPT}")
	message(STATUS "TRUSTED_LINK_OPT   = ${_SGX_TARGET_TRUSTED_LINK_OPT}")
	message(STATUS "TRUSTED_LINK_LIB   = ${_SGX_TARGET_TRUSTED_LINK_LIB}")
	message(STATUS "EDL_PATH           = ${_SGX_TARGET_EDL_PATH}")
	message(STATUS "EDL_INCLUDE        = ${_SGX_TARGET_EDL_INCLUDE}")
	message(STATUS "EDL_OUTPUT_DIR     = ${_SGX_TARGET_EDL_OUTPUT_DIR}")
	message(STATUS "SIGN_CONFIG        = ${_SGX_TARGET_SIGN_CONFIG}")
	message(STATUS "SIGN_KEY           = ${_SGX_TARGET_SIGN_KEY}")
	message(STATUS "==========================================================")
	message(STATUS "")

	##################################################
	# EDL target
	##################################################
	set(_SGX_TARGET_EDL_TRUSTED_OUTPUT
		${_SGX_TARGET_EDL_OUTPUT_DIR}/Enclave_t.c)
	set(_SGX_TARGET_EDL_UNTRUSTED_OUTPUT
		${_SGX_TARGET_EDL_OUTPUT_DIR}/Enclave_u.c)

	file(TOUCH ${_SGX_TARGET_EDL_TRUSTED_OUTPUT})
	file(TOUCH ${_SGX_TARGET_EDL_UNTRUSTED_OUTPUT})

	set(_SGX_TARGET_EDL_SEARCH_PATH_ARG "")
	foreach(item IN LISTS INTEL_SGX_SDK_INCLUDE_DIR _SGX_TARGET_EDL_INCLUDE)
		set(_SGX_TARGET_EDL_SEARCH_PATH_ARG
			${_SGX_TARGET_EDL_SEARCH_PATH_ARG} --search-path "${item}")
	endforeach()

	add_custom_command(OUTPUT ${_SGX_TARGET_EDL_TRUSTED_OUTPUT}
		COMMAND "${INTEL_SGX_EDGER_PATH}"
		--trusted "${_SGX_TARGET_EDL_PATH}"
		${_SGX_TARGET_EDL_SEARCH_PATH_ARG}
		WORKING_DIRECTORY "${_SGX_TARGET_EDL_OUTPUT_DIR}"
		DEPENDS "${_SGX_TARGET_EDL_PATH}"
		COMMENT "Processing EDL for trusted part..."
	)

	add_custom_command(OUTPUT ${_SGX_TARGET_EDL_UNTRUSTED_OUTPUT}
		COMMAND "${INTEL_SGX_EDGER_PATH}"
		--untrusted "${_SGX_TARGET_EDL_PATH}"
		${_SGX_TARGET_EDL_SEARCH_PATH_ARG}
		WORKING_DIRECTORY "${_SGX_TARGET_EDL_OUTPUT_DIR}"
		DEPENDS "${_SGX_TARGET_EDL_PATH}"
		COMMENT "Processing EDL for untrusted part..."
	)

	add_custom_target(${_SGX_TARGET_TARGET_NAME}_edl
		DEPENDS ${_SGX_TARGET_EDL_TRUSTED_OUTPUT}
				${_SGX_TARGET_EDL_UNTRUSTED_OUTPUT})

	set_target_properties(${_SGX_TARGET_TARGET_NAME}_edl
		PROPERTIES FOLDER "${_SGX_TARGET_TARGET_NAME}")

	##################################################
	# Enclave
	##################################################
	list(APPEND _SGX_TARGET_TRUSTED_SOURCE ${_SGX_TARGET_EDL_TRUSTED_OUTPUT})

	add_library(${_SGX_TARGET_TARGET_NAME}_trusted SHARED
		${_SGX_TARGET_TRUSTED_SOURCE})

	#defines:
	target_compile_definitions(${_SGX_TARGET_TARGET_NAME}_trusted
		PRIVATE SECURE_ENCLAVE_ENV INTEL_SGX ${_SGX_TARGET_TRUSTED_DEF})

	#compiler flags:
	target_compile_options(${_SGX_TARGET_TARGET_NAME}_trusted
		PRIVATE ${INTEL_SGX_SDK_TRUSTED_C_FLAGS}
				$<$<COMPILE_LANGUAGE:CXX>:${INTEL_SGX_SDK_TRUSTED_CXX_FLAGS}>
				${_SGX_TARGET_TRUSTED_COMP_OPT})

	#linker flags:
	target_link_options(${_SGX_TARGET_TARGET_NAME}_trusted
		PRIVATE ${INTEL_SGX_SDK_TRUSTED_LINKER_FLAGS}
				${_SGX_TARGET_TRUSTED_LINK_OPT})

	#folder
	set_target_properties(${_SGX_TARGET_TARGET_NAME}_trusted
		PROPERTIES FOLDER "${_SGX_TARGET_TARGET_NAME}")

	set(_SGX_TARGET_TRUSTED_LIB_NAME
		"${CMAKE_SHARED_LIBRARY_PREFIX}${_SGX_TARGET_TARGET_NAME}_trusted$<$<CONFIG:Debug>:${CMAKE_DEBUG_POSTFIX}>")
	set(_SGX_TARGET_TRUSTED_LIB
		"${_SGX_TARGET_TRUSTED_LIB_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")
	set(_SGX_TARGET_TRUSTED_LIB_SIGNED
		"${_SGX_TARGET_TRUSTED_LIB_NAME}.signed${CMAKE_SHARED_LIBRARY_SUFFIX}")

	add_custom_command(TARGET ${_SGX_TARGET_TARGET_NAME}_trusted
		POST_BUILD
		COMMAND  "${INTEL_SGX_SIGNER_PATH}" sign
		-config  "${_SGX_TARGET_SIGN_CONFIG}"
		-key     "${_SGX_TARGET_SIGN_KEY}"
		-enclave "${CMAKE_CURRENT_BINARY_DIR}/${BINARY_SUB_DIR}/${_SGX_TARGET_TRUSTED_LIB}"
		-out     "${CMAKE_CURRENT_BINARY_DIR}/${_SGX_TARGET_TRUSTED_LIB_SIGNED}"
	)

	target_link_libraries(${_SGX_TARGET_TARGET_NAME}_trusted
		${WHOLE_ARCHIVE_FLAG_BEGIN}
		IntelSGX::Trusted::switchless
		IntelSGX::Trusted::rts
		${WHOLE_ARCHIVE_FLAG_END}
		${GROUP_FLAG_BEGIN}
		IntelSGX::Trusted::stdc
		IntelSGX::Trusted::cxx
		IntelSGX::Trusted::service
		IntelSGX::Trusted::key_exchange
		IntelSGX::Trusted::crypto
		IntelSGX::Trusted::file_system
		${_SGX_TARGET_TRUSTED_LINK_LIB}
		${GROUP_FLAG_END}
	)

	add_dependencies(${_SGX_TARGET_TARGET_NAME}_trusted
		${_SGX_TARGET_TARGET_NAME}_edl)

	##################################################
	# Untrusted
	##################################################
	list(APPEND _SGX_TARGET_UNTRUSTED_SOURCE ${_SGX_TARGET_EDL_UNTRUSTED_OUTPUT})

	add_executable(${_SGX_TARGET_TARGET_NAME}
		${_SGX_TARGET_UNTRUSTED_SOURCE})

	#defines:
	target_compile_definitions(${_SGX_TARGET_TARGET_NAME}
		PRIVATE INTEL_SGX
				INTEL_SGX_TRUSTED_LIB="${_SGX_TARGET_TRUSTED_LIB_SIGNED}"
				INTEL_SGX_ECTOKEN="${_SGX_TARGET_TARGET_NAME}_Enclave.token"
				${_SGX_TARGET_UNTRUSTED_DEF})

	#compiler flags:
	target_compile_options(${_SGX_TARGET_TARGET_NAME}
		PRIVATE ${_SGX_TARGET_UNTRUSTED_COMP_OPT})

	#linker flags:
	target_link_options(${_SGX_TARGET_TARGET_NAME}
		PRIVATE ${_SGX_TARGET_UNTRUSTED_LINK_OPT})

	#linker flags:
	set_target_properties(${_SGX_TARGET_TARGET_NAME}
		PROPERTIES FOLDER "${_SGX_TARGET_TARGET_NAME}")

	target_link_libraries(${_SGX_TARGET_TARGET_NAME}
		IntelSGX::Untrusted::Libs
		${_SGX_TARGET_UNTRUSTED_LINK_LIB}
	)

	add_dependencies(${_SGX_TARGET_TARGET_NAME}
		${_SGX_TARGET_TARGET_NAME}_trusted)

ENDFUNCTION()
