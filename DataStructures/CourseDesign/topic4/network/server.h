/*负责：
接收网络请求
解析命令
调用存储引擎*/
#pragma once
#include "../storage/storage.h"

//Redis 服务器本体
class Server {
public:
    explicit Server(StorageEngine& engine);//把存储引擎传进来,服务器后面所有SET/GET/DEL都是调用这个engine
    void start(int port);

private:
    StorageEngine& engine;
};
