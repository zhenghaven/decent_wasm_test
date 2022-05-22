# CLANG    := $(shell which clang)
CLANGXX  := $(shell which clang++)
EMCC     := $(shell which emcc)
WASMLD   := $(shell which wasm-ld)
WASM2WAT := $(shell which wasm2wat)
PWD      := $(shell pwd)


ENTRY_FUNCTION_NAME   := decent_wasm_injected_main
WASM_NATIVE_FUNC_LIST := $(PWD)/../decent_wasm_natives.syms

TARGET_NAME           := wasm32-unknown-emscripten
LIB_SUBDIR            := lib/wasm32-emscripten
SYSROOT_PATH          := /usr/share/emscripten/cache/sysroot


COMPILE_FLAG := -c \
				-O1 \
				--target=$(TARGET_NAME) \
				--sysroot=$(SYSROOT_PATH) \
				-Xclang -iwithsysroot/include/SDL \
				-Xclang -iwithsysroot/include/compat \
				-std=c++17 \
				-D_LIBCPP_ABI_VERSION=2

LINKER_FLAG  := --entry=$(ENTRY_FUNCTION_NAME) \
				--allow-undefined-file=$(WASM_NATIVE_FUNC_LIST) \
				-L$(SYSROOT_PATH)/$(LIB_SUBDIR) \
				-lc \
				-lc++-noexcept \
				-lc++abi-noexcept

all: test.wasm test.wat

%.wat: %.wasm
	$(WASM2WAT) -o $@ $?

%.wasm: %.obj
	$(WASMLD) $(LINKER_FLAG) -o $@ $?

%.obj: %.cpp
	$(EMCC) --version
	$(CLANGXX) $(COMPILE_FLAG) -o $@ $?

clean:
	rm -f *.wat *.wasm *.o *.obj

.PHONY : clean
