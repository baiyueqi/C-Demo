/* UDP 服务器，监听端口并处理客户端 TIME/ECHO/QUIT 命令。 */
//  UDP 服务器：监听端口并处理客户端命令。
#define _POSIX_C_SOURCE 200809L

#include "../include/udp_server.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

static void process_command(const char *cmd, char *response, size_t response_size) {
    // 应用层协议约定，支持 TIME、ECHO:、QUIT，未知命令返回错误。
    if (strcmp(cmd, "TIME") == 0) {
        // 应用层协议：TIME 命令返回服务器当前时间。
        time_t now = time(NULL);
        struct tm tmv;
        localtime_r(&now, &tmv);
        strftime(response, response_size, "%Y-%m-%d %H:%M:%S\n", &tmv);
    } else if (strcmp(cmd, "QUIT") == 0) {
        // 应用层协议：QUIT 命令返回 BYE。
        snprintf(response, response_size, "BYE\n");
    } else if (strncmp(cmd, "ECHO:", 5) == 0) {
        // 应用层协议：ECHO:内容 回显冒号后的内容。
        snprintf(response, response_size, "%s\n", cmd + 5);
    } else {
        // 应用层协议：未知命令返回 ERR:UNKNOWN COMMAND。
        snprintf(response, response_size, "ERR:UNKNOWN COMMAND\n");
    }
}

int run_udp_server(int port) {
    // 使用 SOCK_DGRAM 套接字。
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons((uint16_t)port);

    // 绑定指定端口。
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(sockfd);
        return 1;
    }

    printf("UDP server listening on port %d\n", port);
    fflush(stdout);

    // 循环接收客户端数据报，并能区分不同客户端地址分别响应。
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        char buf[1024];

        // 接收客户端数据报，同时记录客户端地址。
        ssize_t n = recvfrom(sockfd, buf, sizeof(buf) - 1, 0,
                             (struct sockaddr *)&client_addr, &client_len);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("recvfrom");
            continue;
        }

        // 给收到的数据补上字符串结束符，并去掉换行符。
        buf[n] = '\0';
        buf[strcspn(buf, "\r\n")] = '\0';

        char response[512];
        // 根据收到的命令返回结果。
        process_command(buf, response, sizeof(response));

        // 使用 recvfrom 获得的客户端地址回送响应。
        sendto(sockfd, response, strlen(response), 0,
               (struct sockaddr *)&client_addr, client_len);
    }
}
