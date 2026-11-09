# 编译器，默认使用 clang。
CC ?= clang

# 所有目标共同使用的编译选项。
CFLAGS ?= -std=c17 -Wall -Wextra -O2

# 所有可执行文件目标。
TARGETS = task1_signal task2_fifo tcp_server tcp_client udp_server udp_client

.PHONY: all clean

all: $(TARGETS)

task1_signal: main/task1_signal_main.c src/task1_signal.c include/task1_signal.h
	$(CC) $(CFLAGS) main/task1_signal_main.c src/task1_signal.c -o $@

task2_fifo: main/task2_fifo_main.c src/task2_fifo.c include/task2_fifo.h
	$(CC) $(CFLAGS) main/task2_fifo_main.c src/task2_fifo.c -o $@

tcp_server: main/tcp_server_main.c src/tcp_server.c include/tcp_server.h
	$(CC) $(CFLAGS) main/tcp_server_main.c src/tcp_server.c -o $@

tcp_client: main/tcp_client_main.c src/tcp_client.c include/tcp_client.h
	$(CC) $(CFLAGS) main/tcp_client_main.c src/tcp_client.c -o $@

udp_server: main/udp_server_main.c src/udp_server.c include/udp_server.h
	$(CC) $(CFLAGS) main/udp_server_main.c src/udp_server.c -o $@

udp_client: main/udp_client_main.c src/udp_client.c include/udp_client.h
	$(CC) $(CFLAGS) main/udp_client_main.c src/udp_client.c -o $@

clean:
	rm -f $(TARGETS) data_from_parent.txt work3_fifo
