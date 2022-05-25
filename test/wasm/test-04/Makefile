WAT2WASM := $(shell which wat2wasm)

all: test.wasm test.wat

%.wasm: %.wat
	$(WAT2WASM) -o $@ $?

clean:
	rm -f *.wasm

.PHONY : all clean
