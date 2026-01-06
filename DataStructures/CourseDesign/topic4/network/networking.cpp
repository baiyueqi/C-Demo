#include "networking.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int createServer(int port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(fd, (sockaddr*)&addr, sizeof(addr));
    listen(fd, 5);
    return fd;
}

int acceptClient(int serverFd) {
    return accept(serverFd, nullptr, nullptr);
}
