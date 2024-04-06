#include "RedisValue.h"
#include "RedisValueType.h"


RedisValue::RedisValue() noexcept{};

RedisValue::RedisValue(const std::string& value): redisValue(std::make_shared<RedisString>(value)){};

void RedisValue::dump(std::string &out) const {
    redisValue->dump(out); // 调用JsonImpl类的dump函数将Json对象转化为JSON字符串并追加到out中
}
