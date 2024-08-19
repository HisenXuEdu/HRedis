#include "Global.h"

RedisValue::RedisValue() noexcept {};

RedisValue::RedisValue(const std::string& value) : redisValue(std::make_shared<RedisString>(value)) {};
RedisValue::RedisValue(std::string&& value) : redisValue(std::make_shared<RedisString>(std::move(value))) {};
RedisValue::RedisValue(const object& value) : redisValue(std::make_shared<RedisObject>(value)) {};
RedisValue::RedisValue(object&& value) : redisValue(std::make_shared<RedisObject>(std::move(value))) {};


std::string& RedisValueType::getItem(std::string&& s){
    return Statics::statics().emptyString;
}

RedisValue::object& RedisValueType::getItem(RedisValue::object&& o){
    return Statics::statics().emptyObject;
}

RedisValue RedisValue::parse(const std::string& in, std::string& err) {
    // 初始化一个Json解析器
    // RedisValueParser parser { in, 0, err, false};
    // 解析输入字符串已得到Json结果
    // RedisValue result = parser.parseRedisValue(0);

    // // 检查是否有尾随的垃圾字符
    // parser.consumeGarbage();
    // if (parser.failed)
    //     return RedisValue(); // 如果解析失败，返回一个空的Json对象
    // if (parser.i != in.size())
    //     return parser.fail("unexpected trailing " + esc(in[parser.i])); // 如果输入字符串尚有未解析内容，报告错误

    // return result; // 返回解析得到的Json对象
    err = "fail";
    return RedisValue();
}

RedisValue::Type RedisValue::type(){
    return redisValue->type();
}

std::string & RedisValue::stringValue() {
    return redisValue->getItem<std::string>();
}

std::map<std::string, RedisValue> & RedisValue::objectItems()  {
    return redisValue->getItem<std::map<std::string, RedisValue>>();
}


void RedisValue::dump(std::string& out) const {
    redisValue->dump(out);  // 调用JsonImpl类的dump函数将Json对象转化为JSON字符串并追加到out中
}
