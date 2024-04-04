// #include "RedisServer.h"
#include "buttonrpc.hpp"

int fun1(int i, int j){
    return 0;
}
class A{
public:
    int fun1(int i){
        return 0;
    }
};

class RedisServer {
public:
    string handleClient(string receivedData){};
   static RedisServer* getInstance(){
	static RedisServer redis;
	return &redis;
    };
    void start();
};

int main() {
    buttonrpc server;  
    server.as_server(5555);
    // RedisServer::getInstance()->start();
    server.bind("redis_command", &RedisServer::handleClient, RedisServer::getInstance());  //绑定可调用的函数
    int *i;
    // server.bind("redis_command", &A::fun1, i);
    server.run();
}