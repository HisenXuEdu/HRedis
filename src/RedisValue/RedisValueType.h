#ifndef REDISVALUETYPE_H
#define REDISVALUETYPE_H
#include "Dump.h"
#include "RedisValue.h"

class RedisValueType {  // 虚基类
protected:
    friend class RedisValue;
    virtual RedisValue::Type type() const = 0;      // 纯虚函数，获取Tag
    virtual void dump(std::string &out) const = 0;  // 在get key的时候用来返回输出
    // virtual bool equals(const RedisValueType*other) const = 0;
    // virtual bool less(const RedisValueType*other) const = 0;
    template<typename T> T& getItem(){ return getItem(T());}
    virtual std::string& getItem(std::string&& s);
    virtual RedisValue::object& getItem(RedisValue::object&& o);
    
    virtual ~RedisValueType() {}
};

template <RedisValue::Type tag, typename T>
class Value : public RedisValueType {
protected:
    typedef T Type;
    T value;
protected:
    explicit Value(const T &value) : value(value) {}  // 表示显式的，无法隐式转换
    explicit Value(T &&value) : value(std::move(value)) {}
    RedisValue::Type type() const override { return tag; }
    void dump(std::string &out) const override { ::dump(value, out); }  // ::用全局的dump
    // bool equals(const RedisValueType* other) const override {
    //     return value == static_cast<const Value<tag,T>*>(other)->value;
    // }

    // bool less(const RedisValueType* other) const override{
    //     return value < static_cast<const Value<tag, T> *>(other)->value;
    // }
};

class RedisString final : public Value<RedisValue::STRING, std::string> {
    std::string& getItem(std::string&& s) override{ return value; }

public:
    explicit RedisString(const std::string &value) : Value(value) {}  // 显式调用父类的构造函数，父类就是Value<RedisValue::STRING,std::string>，已经填入了模板参数
    explicit RedisString(std::string &&value) : Value(std::move(value)) {}
    void dump(std::string &out) const override { ::dump(value, out); }
};

class RedisObject final : public Value<RedisValue::OBJECT, RedisValue::object> {
    RedisValue::object& getItem() { return value; }
public:
    explicit RedisObject(const RedisValue::object &value) : Value(value) {}
    explicit RedisObject(const RedisValue::object &&value) : Value(std::move(value)) {}
};
#endif