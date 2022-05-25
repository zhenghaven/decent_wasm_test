TESTS    := test-01 \
			test-02 \
			test-03 \
			test-04

TESTS_WAT_FILES   = $(foreach test, $(TESTS), $(test)/test.wasm)
CLEAN_COMMAND     = $(foreach test, $(TESTS), $(MAKE) -C $(test) clean &&) \
					echo "Done with cleaning!"

%.wasm: %.*
	$(MAKE) -C $(shell dirname $@) all

tests: $(TESTS_WAT_FILES)

clean:
	$(CLEAN_COMMAND)

.PHONY : tests clean
