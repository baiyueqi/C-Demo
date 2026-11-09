/* UDP 客户端，向服务器发送 TIME/ECHO/QUIT 等命令并显示响应。 */
//  UDP 客户端：向服务器发送 TIME/ECHO/QUIT 等命令。
#define _POSIX_C_SOURCE 200809L

#include "../include/udp_client.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int run_udp_client(const char *server_ip, int port) {
    // 使用 SOCK_DGRAM 套接字。
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
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

    // 循环读取用户输入，发送到服务器并显示响应。
    char *line = NULL;
    size_t line_size = 0;

    while (1) {
        // 显示提示符并刷新，等待用户输入命令。
        printf("> ");
        fflush(stdout);

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

        // 向指定服务器 IP 和端口发送命令。
        sendto(sockfd, request, strlen(request), 0,
               (struct sockaddr *)&server_addr, sizeof(server_addr));

        char response[1024];
        socklen_t server_len = sizeof(server_addr);
        // 接收服务器返回结果。
        ssize_t n = recvfrom(sockfd, response, sizeof(response) - 1, 0,
                             (struct sockaddr *)&server_addr, &server_len);
        if (n < 0) {
            perror("recvfrom");
            if (quitting) {
                break;
            }
            continue;
        }

        response[n] = '\0';
        // 显示服务器返回结果。
        printf("%s", response);

        // 输入 quit 时退出程序。
        if (quitting) {
            break;
        }
    }

    free(line);
    close(sockfd);
    return 0;
}
