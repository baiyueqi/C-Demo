#include "network/server.h"
#include "storage/storage.h"

int main() {
    StorageEngine engine;
    Server server(engine);
    server.start(6379);
}
