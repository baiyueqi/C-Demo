#include "server.h"
#include "networking.h"
#include "resp.h"
#include <unistd.h>

Server::Server(StorageEngine& e) : engine(e) {}

void Server::start(int port) {
    int sfd = createServer(port);

    while (true) {
        int cfd = acceptClient(sfd);
        if (cfd < 0) continue;

        while (true) {
            char buf[1024] = {0};
            ssize_t n = read(cfd, buf, sizeof(buf));
            if (n <= 0) break;  // 客户端关闭

            auto cmd = Resp::parse(buf);
            if (cmd.empty()) continue;

            std::string reply;
            if (cmd[0] == "SET" && cmd.size() >= 3) {
                engine.set(cmd[1], cmd[2]);
                reply = Resp::simple("OK");
            } 
            else if (cmd[0] == "GET" && cmd.size() >= 2) {
                auto v = engine.get(cmd[1]);
                reply = v ? Resp::bulk(*v) : Resp::nullBulk();
            } 
            else if (cmd[0] == "DEL" && cmd.size() >= 2) {
                reply = Resp::simple(engine.del(cmd[1]) ? "1" : "0");
            } 
            else {
                reply = Resp::simple("ERR");
            }

            write(cfd, reply.c_str(), reply.size());
        }

        close(cfd);
    }
}

