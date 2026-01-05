#include "network/server.h"

int main() {
    Server server(6379);
    server.start();
    return 0;
}
