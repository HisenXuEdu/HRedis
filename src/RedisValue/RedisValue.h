#ifndef REDISVALUE_H 
#define REDISVALUE_H
#include<string>
#include<memory>

class RedisValueType;

class RedisValue{
public:
    enum Type{
        NUL,NUMBER,BOOL,STRING,ARRAY,OBJECT
    };
public:
    // 构造函数
    RedisValue() noexcept;
    RedisValue(std::nullptr_t) noexcept;
    RedisValue(const std::string& value);
    RedisValue(std::string&& value);
    RedisValue(const char* value);

    RedisValue(void*) = delete; // 禁止从 void* 构造
    // RedisValue(const array&value);
    // RedisValue(array&& values);
    // RedisValue(const object& values);
    // RedisValue(object && values);
        

    // 序列化函数
    void dump(std::string &out) const;
    std::string dump() const{
        std::string out;
        dump(out);
        return out;
    }

private:
    std::shared_ptr<RedisValueType> redisValue; // 指向实际存储的智能指针
};


#endif