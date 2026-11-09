//  TCP 服务器入口，负责解析端口并调用服务器实现。
#include "../include/tcp_server.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    // 检查参数数量：程序名、端口。
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }

    // 将端口字符串转换为整数，并检查范围。
    int port = atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Invalid port: %s\n", argv[1]);
        return 1;
    }

    return run_tcp_server(port);
}
