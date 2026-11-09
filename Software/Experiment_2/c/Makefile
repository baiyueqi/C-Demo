CC ?= clang
CFLAGS ?= -std=c17 -Wall -Wextra -Wpedantic -O2
TIDY ?= clang-tidy
BIN_DIR ?= /tmp/exp2
ANALYZE_FLAGS ?= -Wno-unused-command-line-argument
TIDY_CHECKS ?= clang-analyzer-*,clang-diagnostic-*,-clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling
CLANG_INCLUDES := $(shell printf '' | clang -E -x c - -v 2>&1 | sed -n '/#include <...> search starts here:/,/End of search list./p' | grep '^ /' | sed 's/^ //' | tr '\n' ' ')
TIDY_EXTRA := $(foreach inc,$(CLANG_INCLUDES),--extra-arg=-isystem$(inc))

.PHONY: all analyze analyze-vuln analyze-fixed tidy tidy-vuln tidy-fixed run-fixed clean

all: vulnerable fixed

vulnerable: vulnerable.c
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) vulnerable.c -o $(BIN_DIR)/vulnerable

fixed: fixed.c
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) fixed.c -o $(BIN_DIR)/fixed

analyze-vuln:
	$(CC) $(ANALYZE_FLAGS) --analyze -Xanalyzer -analyzer-output=text vulnerable.c

analyze-fixed:
	$(CC) $(ANALYZE_FLAGS) --analyze -Xanalyzer -analyzer-output=text fixed.c

analyze: analyze-vuln analyze-fixed

tidy-vuln:
	@echo "[clang-tidy] scanning vulnerable.c"
	@$(TIDY) -p . vulnerable.c $(TIDY_EXTRA) -checks=$(TIDY_CHECKS)

tidy-fixed:
	@echo "[clang-tidy] scanning fixed.c"
	@$(TIDY) -p . fixed.c $(TIDY_EXTRA) -checks=$(TIDY_CHECKS)

tidy: tidy-vuln tidy-fixed

run-fixed: fixed
	$(BIN_DIR)/fixed

clean:
	rm -f $(BIN_DIR)/vulnerable $(BIN_DIR)/fixed
