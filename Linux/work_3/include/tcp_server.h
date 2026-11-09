//  TCP 服务器接口声明。
#ifndef TCP_SERVER_H
#define TCP_SERVER_H

// 在指定端口启动 TCP 服务器，返回非 0 表示启动失败。
int run_tcp_server(int port);

#endif
