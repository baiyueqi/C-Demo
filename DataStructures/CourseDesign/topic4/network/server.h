#pragma once
#include "storage/storage.h"

class Server {
public:
    explicit Server(int port);
    void start();

private:
    int port;
    StorageEngine storage;
};
