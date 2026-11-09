/* TCP 客户端，向服务器发送 TIME/ECHO/QUIT 等命令并显示响应。 */
//  TCP 客户端：连接服务器并发送 TIME/ECHO/QUIT 等命令。
#define _POSIX_C_SOURCE 200809L

#include "../include/tcp_client.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static int send_all(int sockfd, const char *data) {
    // 循环发送，直到所有字节都发送完成。
    size_t remaining = strlen(data);
    const char *p = data;

    while (remaining > 0) {
        ssize_t n = send(sockfd, p, remaining, 0);
        if (n <= 0) {
            return -1;
        }
        p += n;
        remaining -= (size_t)n;
    }

    return 0;
}

int run_tcp_client(const char *server_ip, int port) {
    // 使用 SOCK_STREAM 套接字。
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons((uint16_t)port);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) != 1) {
        fprintf(stderr, "Invalid server IP: %s\n", server_ip);
        close(sockfd);
        return 1;
    }

    // 连接指定服务器 IP 和端口。
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sockfd);
        return 1;
    }

    // 忽略 SIGPIPE，避免向已关闭连接发送数据时终止进程。
    signal(SIGPIPE, SIG_IGN);

    // 将 socket 包装成标准 IO 流，便于按行读取响应。
    FILE *server_in = fdopen(sockfd, "r");
    if (server_in == NULL) {
        perror("fdopen");
        close(sockfd);
        return 1;
    }

    char *line = NULL;
    size_t line_size = 0;

    while (1) {
        // 显示提示符并刷新，等待用户输入命令。
        printf("> ");
        fflush(stdout);

        // 从标准输入读取用户命令。
        if (getline(&line, &line_size, stdin) < 0) {
            printf("\n");
            break;
        }

        // 去掉输入行末尾的换行符。
        line[strcspn(line, "\r\n")] = '\0';

        // 应用层协议：用户输入 quit 时转换为 QUIT 命令。
        int quitting = (strcmp(line, "quit") == 0 || strcmp(line, "QUIT") == 0);
        char request[1024];
        // 应用层协议：每条命令后添加换行符，作为消息结束标志。
        snprintf(request, sizeof(request), "%s\n", quitting ? "QUIT" : line);

        // 把命令字符串发送给服务器。
        if (send_all(sockfd, request) < 0) {
            break;
        }

        char *response = NULL;
        size_t response_size = 0;
        // 接收并显示服务器返回结果。
        if (getline(&response, &response_size, server_in) < 0) {
            free(response);
            break;
        }

        printf("%s", response);
        free(response);

        // 输入 quit 时关闭连接并退出。
        if (quitting) {
            break;
        }
    }

    free(line);
    fclose(server_in);
    return 0;
}
