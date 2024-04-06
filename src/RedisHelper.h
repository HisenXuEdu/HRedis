#ifndef REDISHELPER_H
#define REDISHELPER_H
#include <memory>
#include <string>
#include <vector>
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
    std::string get(const std::string& key);


};

#endif