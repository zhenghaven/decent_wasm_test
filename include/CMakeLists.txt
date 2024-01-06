# Copyright (c) 2024 Haofan Zheng
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.


option(
	DECENTWASMRUNTIME_INSTALL_HEADERS
	"Install Decent WASM Runtime headers."
	ON
)

add_library(DecentWasmRuntime INTERFACE)
target_include_directories(DecentWasmRuntime INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if(DECENTWASMRUNTIME_INSTALL_HEADERS)

	file(GLOB headers "DecentWasmRuntime/*.hpp")

	install(
		FILES ${headers}
		DESTINATION include/DecentWasmRuntime
		PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
	)

	# add_subdirectory(DecentWasmRuntime)

endif(DECENTWASMRUNTIME_INSTALL_HEADERS)

