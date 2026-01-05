// 修改 server.cpp
#include "server.h"
#include "networking.h"
#include "resp.h"
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <iomanip>

Server::Server(int port) : port(port) {}

void Server::start() {
    int serverFd = createServer(port);
    std::cout << "Server started on port " << port << std::endl;

    while (true) {
        int clientFd = accept(serverFd, nullptr, nullptr);
        std::string req = recvData(clientFd);
        
        // ========== 调试输出开始 ==========
        std::cout << "\n=== 收到请求 ===" << std::endl;
        std::cout << "原始字节(" << req.size() << "): ";
        for (size_t i = 0; i < req.size(); i++) {
            unsigned char c = req[i];
            if (c == '\r') std::cout << "\\r";
            else if (c == '\n') std::cout << "\\n";
            else if (c >= 32 && c <= 126) std::cout << c;
            else std::cout << "\\x" << std::hex << (int)c << std::dec;
        }
        std::cout << std::endl;
        
        std::cout << "十六进制: ";
        for (size_t i = 0; i < req.size(); i++) {
            printf("%02x ", (unsigned char)req[i]);
        }
        std::cout << std::endl;
        // ========== 调试输出结束 ==========
        
        auto args = RESP::parse(req);
        std::string reply = "-ERR\r\n";

        if (args.size() >= 2) {
            if (args[0] == "GET") reply = storage.get(args[1]);
            else if (args[0] == "DEL") reply = storage.del(args[1]);
        }
        if (args.size() >= 3 && args[0] == "SET") {
            reply = storage.set(args[1], args[2]);
        }
        
        std::cout << "解析结果: ";
        for (const auto& arg : args) {
            std::cout << "[" << arg << "] ";
        }
        std::cout << "\n回复: " << reply << std::endl;

        sendData(clientFd, reply);
        close(clientFd);
    }
}
