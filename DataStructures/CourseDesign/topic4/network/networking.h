#pragma once
#include <string>

int createServer(int port);
std::string recvData(int clientFd);
void sendData(int clientFd, const std::string& data);
