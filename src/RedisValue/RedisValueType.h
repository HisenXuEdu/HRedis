#ifndef REDISVALUETYPE_H
#define REDISVALUETYPE_H
#include "RedisValue.h"
#include "Dump.h"

class RedisValueType{ //虚基类
protected:
    friend class RedisValue;
    // virtual RedisValue::Type type() const = 0;  //纯虚函数，这些函数是每一个子类都要实现的，否则无法构造对象
    virtual void dump(std::string& out) const = 0; //再get key的时候用来返回输出
    // virtual std::string &stringValue() ;
    virtual ~RedisValueType(){}
};



template<RedisValue::Type tag,typename T>  
class Value : public RedisValueType{
protected:
    T value;
protected:
    explicit Value(const T& value) : value(value){}  //表示显式的，无法隐式转换
    explicit Value(T&&value) : value(std::move(value)){}
    // RedisValue::Type type() const override{
    //     return tag;
    // }
    //??? 这里为什么一定要加::
    void dump(std::string&out) const override{::dump(value,out);}
};


//实现每一种类型的RedisValue
class RedisString final:public Value<RedisValue::STRING,std::string>{  //final代表这个类不想被继承，或者虚函数不想被重写
    // std::string & stringValue()override { return value;}
public:
    explicit RedisString(const std::string& value): Value(value){}  //显式调用父类的构造函数，父类就是Value<RedisValue::STRING,std::string>，已经填入了模板参数
    explicit RedisString(std::string&& value) : Value(std::move(value)) {}
};


#endif