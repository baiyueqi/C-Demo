//  TCP 客户端接口声明。
#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

/*
 * 连接指定 TCP 服务器并进入命令交互循环。
 * server_ip: 服务器 IPv4 地址字符串
 * port:      服务器端口
 */
int run_tcp_client(const char *server_ip, int port);

#endif
