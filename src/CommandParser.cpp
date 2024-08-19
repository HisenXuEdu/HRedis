#include "CommandParser.h"

// 静态成员变量的初始化
std::shared_ptr<RedisHelper> CommandParser::redisHelper = std::make_shared<RedisHelper>();
// 饿汉模式所以直接创建这个对象，全局只有一个redisHelper对象

std::string SetParser::parse(std::vector<std::string>& tokens) {
    if (tokens.size() < 3 || tokens.size() > 4) {
        return "wrong number of arguments for SET.";
    }
    if (tokens.size() == 4) {
        if (tokens.back() == "NX") {
            return redisHelper->set(tokens[1], tokens[2], NX);
        } else if (tokens.back() == "XX") {
            return redisHelper->set(tokens[1], tokens[2], XX);
        }
    }
    return redisHelper->set(tokens[1], tokens[2]);
}

std::string GetParser::parse(std::vector<std::string>& tokens) {
    if (tokens.size() < 2) {
        return "wrong number of arguments for GET.";
    }
    return redisHelper->get(tokens[1]);
}

std::string HSetParser::parse(std::vector<std::string>& tokens) {
    if (tokens.size() < 4 || tokens.size() % 2) {
        return "wrong number of arguments for Hset.";
    }
    return redisHelper->hset(tokens[1], std::vector<std::string>(tokens.begin() + 2, tokens.end()));
}

std::string HGetParser::parse(std::vector<std::string>& tokens) {
    if (tokens.size() != 3) {
        return "wrong number of arguments for Hset.";
    }
    return redisHelper->hget(tokens[1], tokens[2]);
}
