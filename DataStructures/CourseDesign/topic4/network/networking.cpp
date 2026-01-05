#include "networking.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int createServer(int port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    bind(fd, (sockaddr*)&addr, sizeof(addr));
    listen(fd, 5);
    return fd;
}

std::string recvData(int clientFd) {
    char buf[1024]{};
    int n = read(clientFd, buf, sizeof(buf));
    return n > 0 ? std::string(buf, n) : "";
}

void sendData(int clientFd, const std::string& data) {
    write(clientFd, data.c_str(), data.size());
}
