#ifndef REDISVALUE_H 
#define REDISVALUE_H
#include<string>
#include<memory>

class RedisValue{
    int a=1;
public:
    // 构造函数
    RedisValue() noexcept;
    RedisValue(std::nullptr_t) noexcept;
    RedisValue(const std::string& value);
    RedisValue(std::string&& value);
    RedisValue(const char* value);
    // RedisValue(const array&value);
    // RedisValue(array&& values);
    // RedisValue(const object& values);
    // RedisValue(object && values);

private:
    // std::shared_ptr<RedisValueType> redisValue; // 指向实际存储的智能指针
};


#endif