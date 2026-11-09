CC ?= clang
CFLAGS ?= -std=c17 -O0 -g -fno-stack-protector -U_FORTIFY_SOURCE
LDFLAGS ?= -Wl,-z,execstack
TARGET := buffer_overflow_linux

.PHONY: all clean run

all: $(TARGET)

$(TARGET): buffer_overflow_linux.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
