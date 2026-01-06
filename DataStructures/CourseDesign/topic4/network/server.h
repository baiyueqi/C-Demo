#pragma once
#include "../storage/storage.h"

class Server {
public:
    explicit Server(StorageEngine& engine);
    void start(int port);

private:
    StorageEngine& engine;
};
