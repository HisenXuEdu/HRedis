#ifndef REDISVALUE_H
#define REDISVALUE_H
#include <string>
#include <map>
#include "memory"

class RedisValueType;

class RedisValue {
public:
    enum Type { NUL, NUMBER, BOOL, STRING, ARRAY, OBJECT };
    typedef std::map<std::string, RedisValue> object;

public:
    // 构造函数
    RedisValue() noexcept;
    RedisValue(std::nullptr_t) noexcept;
    RedisValue(const std::string& value);
    RedisValue(std::string&& value);
    RedisValue(const char* value);
    RedisValue(const object& value);
    RedisValue(object&& value);
    RedisValue(void*) = delete;  // 禁止从 void* 构造

    // // 解析 JSON 文本的静态函数
    static RedisValue parse(const std::string&in, std::string& err);
    // static RedisValue parse(const char* in, std::string& err);

    // 获取值的函数
    std::string& stringValue() ;
    object &objectItems() ;

    Type type();

    // 序列化函数
    void dump(std::string& out) const;

    std::string dump() const {
        std::string out;
        dump(out);
        return out;
    }

private:
    std::shared_ptr<RedisValueType> redisValue;  // 指向实际存储的智能指针
};

#endif