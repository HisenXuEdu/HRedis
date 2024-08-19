#ifndef DUMP_H
#define DUMP_H
#include<string>
#include<cmath>
#include"RedisValue.h"

//因为在一个类型中可能会调用别的类型的dump，所以不能设计成成员函数


// 用于将字符串值进行转义处理并追加到输出字符串中，static限定只在当前文件可见
static void dump(const std::string &value, std::string &out) {
    out += '"';
    for (size_t i = 0; i < value.length(); i++) {
        const char ch = value[i];
        // 根据字符进行相应的转义处理
        switch (ch) {
            case '\\': out += "\\\\"; break;
            case '"':  out += "\\\""; break;
            case '\b': out += "\\b"; break;
            case '\f': out += "\\f"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (static_cast<uint8_t>(ch) <= 0x1f) {
                    char buf[8];
                    snprintf(buf, sizeof buf, "\\u%04x", ch); // 对控制字符进行Unicode转义
                    out += buf;
                } else {
                    out += ch;
                }
        }
    }
    out += '"';
}

static void dump(const RedisValue::object &value, std::string &out){
    bool first=1;
    out+="{";
    for(const auto &item:value){
        if(!first) out+=",";
        dump(item.first, out);
        out+=":";
        item.second.dump(out);
        first=false;
    }
    out+="}";
}



#endif