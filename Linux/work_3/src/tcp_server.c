/* TCP 服务器，监听端口并处理客户端 TIME/ECHO/QUIT 命令。 */
//  TCP 服务器：监听端口并处理客户端命令。
#define _POSIX_C_SOURCE 200809L

#include "../include/tcp_server.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
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

static void handle_client(int connfd) {
    // 单个客户端连接：循环读取命令行，处理后发送响应。
    FILE *in = fdopen(connfd, "r");
    if (in == NULL) {
        perror("fdopen");
        close(connfd);
        return;
    }

    char *line = NULL;
    size_t line_size = 0;

    // 接收客户端发送的命令字符串。
    while (getline(&line, &line_size, in) != -1) {
        // 去掉命令末尾换行符后交给 process_command 处理。
        line[strcspn(line, "\r\n")] = '\0';

        char response[512];
        // 根据命令返回对应结果。
        process_command(line, response, sizeof(response));

        // 发送服务器生成的响应。
        if (send(connfd, response, strlen(response), 0) < 0) {
            break;
        }

        if (strcmp(line, "QUIT") == 0) {
            break;
        }
    }

    free(line);
    fclose(in);
}

int run_tcp_server(int port) {
    // 使用 SOCK_STREAM 套接字，建立面向连接的 TCP 服务器。
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        perror("socket");
        return 1;
    }

    // 允许端口复用，避免重启服务器时出现地址占用问题。
    int reuse = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt");
        close(listenfd);
        return 1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons((uint16_t)port);

    // 绑定指定端口并监听客户端连接。
    if (bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(listenfd);
        return 1;
    }

    if (listen(listenfd, 16) < 0) {
        perror("listen");
        close(listenfd);
        return 1;
    }

    // 忽略子进程结束和写断管信号，保持服务器稳定运行。
    signal(SIGCHLD, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);

    printf("TCP server listening on port %d\n", port);
    fflush(stdout);

    // 支持多个客户端依次或并发访问，客户端断开后继续服务其他客户端。
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int connfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_len);
        if (connfd < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("accept");
            continue;
        }

        // 为每个客户端连接创建一个子进程处理，实现并发访问。
        // TCP 服务器采用多进程实现并发。
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            close(connfd);
            continue;
        }

        if (pid == 0) {
            // 子进程关闭监听 socket，只处理当前连接。
            close(listenfd);
            handle_client(connfd);
            _exit(0);
        }

        close(connfd);
    }
}
