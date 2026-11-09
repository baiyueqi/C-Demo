//  UDP 客户端接口声明。
#ifndef UDP_CLIENT_H
#define UDP_CLIENT_H

/*
 * 向指定 UDP 服务器发送命令并显示响应。
 * server_ip: 服务器 IPv4 地址字符串
 * port:      服务器端口
 */
int run_udp_client(const char *server_ip, int port);

#endif
