# 实验（3）进程管理及进程间通信

本目录包含实验要求的 C 语言实现。

## 目录结构

```text
Linux/work_3/
├── Makefile
├── include/
│   ├── task1_signal.h
│   ├── task2_fifo.h
│   ├── tcp_client.h
│   ├── tcp_server.h
│   ├── udp_client.h
│   └── udp_server.h
├── src/
│   ├── task1_signal.c
│   ├── task2_fifo.c
│   ├── tcp_client.c
│   ├── tcp_server.c
│   ├── udp_client.c
│   └── udp_server.c
└── main/
    ├── task1_signal_main.c
    ├── task2_fifo_main.c
    ├── tcp_client_main.c
    ├── tcp_server_main.c
    ├── udp_client_main.c
    └── udp_server_main.c
```

## 文件说明

- `task1_signal`：第 1 题，父进程写文件后用 SIGUSR1 通知子进程读取显示。
- `task2_fifo`：第 2 题，用命名管道实现第 1 题的通信。
- `tcp_server` / `tcp_client`：第 3 题 TCP 服务器与客户端。
- `udp_server` / `udp_client`：第 3 题 UDP 服务器与客户端。

## 编译

```sh
make
```

## 运行

第 1 题：

```sh
./task1_signal
```

第 2 题：

```sh
./task2_fifo
```

TCP：

```sh
./tcp_server 8888
./tcp_client 127.0.0.1 8888
```

UDP：

```sh
./udp_server 8888
./udp_client 127.0.0.1 8888
```

客户端支持命令示例：

```text
TIME
ECHO:hello
QUIT
```
