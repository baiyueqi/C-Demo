//  UDP 客户端入口，负责解析参数并调用客户端实现。
#include "../include/udp_client.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    // 检查参数数量：程序名、服务器 IP、端口。
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <port>\n", argv[0]);
        return 1;
    }

    // 将端口字符串转换为整数，并检查范围。
    int port = atoi(argv[2]);
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Invalid port: %s\n", argv[2]);
        return 1;
    }

    return run_udp_client(argv[1], port);
}
