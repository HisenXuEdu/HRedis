#include "RedisServer.h"
#include "buttonrpc.hpp"

int main() {
    buttonrpc server;  
    server.as_server(5555);
    RedisServer::getInstance()->start();
    server.bind("redis_command", &RedisServer::handleClient, RedisServer::getInstance());  //绑定可调用的函数
    server.run();
}