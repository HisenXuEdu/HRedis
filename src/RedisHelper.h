#ifndef REDISHELPER_H
#define REDISHELPER_H
#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include "SkipList.h" 
#include "RedisValue/RedisValue.h"
#define DEFAULT_DB_FOLDER "data_files"
#define DATABASE_FILE_NAME "db"
#define DATABASE_FILE_NUMBER 15

class RedisHelper{
private:
    std::string dataBaseIndex="0"; //当前数据库索引
    std::shared_ptr<SkipList<std::string, RedisValue>> redisDataBase = nullptr;
public:
    RedisHelper();
    ~RedisHelper();


private:
    //从文件中加载数据  持久性保存数据
    void loadData(std::string loadPath);  
    std::string getFilePath();

public:
    void flush(); //写入文件 
    //选择数据库
    std::string select(int index);

    // 字符串操作命令
    std::string set(const std::string& key, const RedisValue& value,const SET_MODEL model=NONE);
    std::string setnx(const std::string& key, const RedisValue& value);
    std::string setex(const std::string& key, const RedisValue& value);

    // 获取键值
    std::string get(const std::string& key);

    // 值递增/递减
    std::string incr(const std::string& key);

    std::string incrby(const std::string& key,int increment);

    std::string incrbyfloat(const std::string&key,double increment);

    // 同样，递减使用decr、decrby命令。
    std::string decr(const std::string&key);

    std::string decrby(const std::string&key,int increment);

    // 批量存放键值
    std::string mset(std::vector<std::string>&items);

    // 批量获取获取键值
    std::string mget(std::vector<std::string>&keys);

    // 获取值长度
    std::string strlen(const std::string& key);

    // 追加内容
    std::string append(const std::string&key,const std::string &value);



    //哈希表操作
    // HSET key field value：向哈希表中添加一个字段及其值。
    // HGET key field：获取哈希表中指定字段的值。
    // HDEL key field：删除哈希表 key 中的一个或多个指定字段。
    // HKEYS key：获取哈希表中的所有字段名。
    // HVALS key：获取哈希表中的所有值。
    std::string hset(const std::string&key,const std::vector<std::string>&filed);
    std::string hget(const std::string&key,const std::string&filed);
    std::string hdel(const std::string&key,const std::vector<std::string>&filed);
    std::string hkeys(const std::string&key);
    std::string hvals(const std::string&key);

};

#endif