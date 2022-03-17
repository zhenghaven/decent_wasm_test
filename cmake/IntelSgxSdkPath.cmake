cmake_minimum_required(VERSION 3.10)

include_guard()

message(STATUS "Setting Intel SGX Variables...")

if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
	set(WIN_ARCHI_STR "win32")
	set(LINUX_LIB_ARCHI_STR "lib") #for libs
	set(LINUX_BIN_ARCHI_STR "x86") #for bins, e.g. signing tool.
else()
	set(WIN_ARCHI_STR "x64")
	set(LINUX_LIB_ARCHI_STR "lib64") #for libs
	set(LINUX_BIN_ARCHI_STR "x64") #for bins, e.g. signing tool.
endif()

##################################################
# SGX SDK Path
##################################################

####################
# Windows OS
####################
If(WIN32)
	if((NOT DEFINED ENV{SGXSDKInstallPath}) OR (NOT EXISTS "$ENV{SGXSDKInstallPath}"))
		message(FATAL_ERROR "Intel SGX SDK is not installed properly!")
	else()
		get_filename_component(INTEL_SGX_SDK_PATH "$ENV{SGXSDKInstallPath}" ABSOLUTE)
	endif()

####################
# Linux OS
####################
elseif(UNIX AND NOT APPLE)
	if((NOT DEFINED ENV{SGX_SDK}))
		set(INTEL_SGX_SDK_PATH "/opt/intel/sgxsdk")
	else()
		set(INTEL_SGX_SDK_PATH "$ENV{SGX_SDK}")
	endif()

	if(NOT EXISTS ${INTEL_SGX_SDK_PATH})
		message(FATAL_ERROR "Intel SGX SDK is not installed properly!")
	else()
		get_filename_component(INTEL_SGX_SDK_PATH "${INTEL_SGX_SDK_PATH}" ABSOLUTE)
	endif()

####################
# Other OS (unsupported OS)
####################
else()
	message(FATAL_ERROR "OS not supported by Intel SGX!")
endif()

##################################################
# SGX Tools Path
##################################################

####################
# Windows OS
####################
if(MSVC)
	set(INTEL_SGX_EDGER_PATH "${INTEL_SGX_SDK_PATH}/bin/win32/Release/sgx_edger8r.exe")
	set(INTEL_SGX_SIGNER_PATH "${INTEL_SGX_SDK_PATH}/bin/win32/Release/sgx_sign.exe")

####################
# Linux OS
####################
elseif(UNIX AND NOT APPLE)
	set(INTEL_SGX_EDGER_PATH "${INTEL_SGX_SDK_PATH}/bin/${LINUX_BIN_ARCHI_STR}/sgx_edger8r")
	set(INTEL_SGX_SIGNER_PATH "${INTEL_SGX_SDK_PATH}/bin/${LINUX_BIN_ARCHI_STR}/sgx_sign")
endif()

get_filename_component(INTEL_SGX_EDGER_PATH ${INTEL_SGX_EDGER_PATH} ABSOLUTE)
get_filename_component(INTEL_SGX_SIGNER_PATH ${INTEL_SGX_SIGNER_PATH} ABSOLUTE)

############################
# SGX Lib Path
############################
if(WIN32)

	#headers:
	set(INTEL_SGX_SDK_INCLUDE_DIR "${INTEL_SGX_SDK_PATH}/include")
	#C Flags:
	set(INTEL_SGX_SDK_TRUSTED_C_FLAGS "")
	#CXX Flags:
	set(INTEL_SGX_SDK_TRUSTED_CXX_FLAGS "")
	#Linker Flags:
	set(INTEL_SGX_SDK_TRUSTED_LINKER_FLAGS /NODEFAULTLIB /NOENTRY)

elseif(UNIX)

	#headers:
	set(INTEL_SGX_SDK_INCLUDE_DIR "${INTEL_SGX_SDK_PATH}/include")
	#C Flags:
	set(INTEL_SGX_SDK_TRUSTED_C_FLAGS
		-nostdinc -fvisibility=hidden -fpie -fstack-protector)
	#CXX Flags:
	set(INTEL_SGX_SDK_TRUSTED_CXX_FLAGS
		-nostdinc++)
	#Linker Flags:
	set(INTEL_SGX_SDK_TRUSTED_LINKER_FLAGS
		"-Wl,--no-undefined" "-nostdlib" "-nodefaultlibs" "-nostartfiles"
		"-Wl,-Bstatic" "-Wl,-Bsymbolic" "-Wl,--no-undefined"
		"-Wl,-pie,-eenclave_entry" "-Wl,--export-dynamic"
		"-Wl,--defsym,__ImageBase=0")

endif()

##################################################
# SGX VARIABLES TO BE EXPORTED
##################################################

# ${INTEL_SGX_SDK_PATH}
# ${INTEL_SGX_SDK_INCLUDE_DIR}

# ${INTEL_SGX_EDGER_PATH}
# ${INTEL_SGX_SIGNER_PATH}

# ${INTEL_SGX_SDK_TRUSTED_C_FLAGS}
# ${INTEL_SGX_SDK_TRUSTED_CXX_FLAGS}
# ${INTEL_SGX_SDK_TRUSTED_LINKER_FLAGS}


set(INTEL_SGX_SDK_PATH        ${INTEL_SGX_SDK_PATH})
set(INTEL_SGX_SDK_INCLUDE_DIR ${INTEL_SGX_SDK_INCLUDE_DIR})

set(INTEL_SGX_EDGER_PATH  ${INTEL_SGX_EDGER_PATH})
set(INTEL_SGX_SIGNER_PATH ${INTEL_SGX_SIGNER_PATH})

set(INTEL_SGX_SDK_TRUSTED_C_FLAGS
	${INTEL_SGX_SDK_TRUSTED_C_FLAGS})
set(INTEL_SGX_SDK_TRUSTED_CXX_FLAGS
	${INTEL_SGX_SDK_TRUSTED_CXX_FLAGS})
set(INTEL_SGX_SDK_TRUSTED_LINKER_FLAGS
	${INTEL_SGX_SDK_TRUSTED_LINKER_FLAGS})

message(STATUS "Finished setting Intel SGX Variables.")
message(STATUS "")
