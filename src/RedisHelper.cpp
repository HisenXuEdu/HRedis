#include "RedisHelper.h"


RedisHelper::RedisHelper(){
    redisDataBase = std::make_shared<SkipList<std::string, RedisValue>>();
};
RedisHelper::~RedisHelper(){};



std::string RedisHelper::set(const std::string& key, const RedisValue& value,const SET_MODEL model){
    if(model==XX){
        //return setex(key,value);
    }else if(model==NX){

    }else{
        auto currentNode=redisDataBase->searchItem(key);
        if(currentNode==nullptr){
            setnx(key,value);
        }else{
            setex(key,value);
        }
    }
    return "OK";
}

std::string RedisHelper::setnx(const std::string& key, const RedisValue& value){
    auto currentNode=redisDataBase->searchItem(key);
    if(currentNode!=nullptr){
        return "key: "+ key +"  exists!";
    }else{
        redisDataBase->addItem(key,value);
    }
    return "OK";
}

std::string RedisHelper::setex(const std::string& key, const RedisValue& value){
    return "OK";
}