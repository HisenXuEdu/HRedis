#include "CommandParser.h"


// 静态成员变量的初始化
std::shared_ptr<RedisHelper> CommandParser::redisHelper = std::make_shared<RedisHelper>();
//饿汉模式所以直接创建这个对象，全局只有一个redisHelper对象


std::string SetParser::parse(std::vector<std::string>& tokens){
    if (tokens.size() < 3 || tokens.size() > 4) {
        return "wrong number of arguments for SET.";
    }
    if(tokens.size() == 4){
        if (tokens.back() == "NX") {
            // return redisHelper->set(tokens[1], tokens[2], NX);
        } else if (tokens.back() == "XX") {
            // return redisHelper->set(tokens[1], tokens[2], XX);
        }
    }
    return redisHelper->set(tokens[1], tokens[2]);
}