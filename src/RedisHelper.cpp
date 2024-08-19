#include "RedisHelper.h"

#include <map>

#include "FileCreator.h"

void RedisHelper::flush() {
    // 打开文件并覆盖写入
    std::string filePath = getFilePath();
    std::ofstream outputFile(filePath);
    // 检查文件是否打开成功
    if (!outputFile) {
        std::cout << "文件：" << filePath << "打开失败" << std::endl;
        return;
    }
    auto currentNode = redisDataBase->gethead();
    while (currentNode != nullptr) {  // 写入磁盘的是key和value，而不是节点结构。
        std::string key = currentNode->key;
        RedisValue value = currentNode->value;
        if (!key.empty())  // 判断string是否为空
            outputFile << key << ":" << value.dump() << std::endl;
        currentNode = currentNode->forward[0];
    }
    outputFile.close();
}

std::string RedisHelper::getFilePath() {
    std::string folder = DEFAULT_DB_FOLDER;
    std::string fileName = DATABASE_FILE_NAME;
    std::string filePath = folder + "/" + fileName + dataBaseIndex;  // 文件路径
    return filePath;
}

void RedisHelper::loadData(std::string loadPath) { redisDataBase->loadFile(loadPath); }

std::string RedisHelper::select(int index) {
    if (index < 0 || index > 15) {
        return "database index out of range.";
    }
    flush();
    redisDataBase = std::make_shared<SkipList<std::string, RedisValue>>();
    dataBaseIndex = std::to_string(index);
    std::string filePath = getFilePath();

    loadData(filePath);
    return "OK";
}

RedisHelper::RedisHelper() {
    redisDataBase = std::make_shared<SkipList<std::string, RedisValue>>();
    // 每次检查并创建磁盘db
    FileCreator::createFolderAndFiles(DEFAULT_DB_FOLDER, DATABASE_FILE_NAME, DATABASE_FILE_NUMBER);
    std::string filePath = getFilePath();
    loadData(filePath);
};
RedisHelper::~RedisHelper() { flush(); };

// 在声明总写了默认值，这里就不用写了。
std::string RedisHelper::set(const std::string& key, const RedisValue& value, const SET_MODEL model) {
    if (model == XX) {
        return setex(key, value);
    } else if (model == NX) {
        return setnx(key, value);
    } else {
        auto currentNode = redisDataBase->searchItem(key);
        if (currentNode == nullptr) {
            setnx(key, value);
        } else {
            setex(key, value);
        }
    }
    return "OK";
}

std::string RedisHelper::setnx(const std::string& key, const RedisValue& value) {
    // todo 这里的实现结构并不是很好，因为每次都要调用很多遍searchItem
    auto currentNode = redisDataBase->searchItem(key);
    if (currentNode != nullptr) {
        return "key: " + key + "  exists!";
    } else {
        redisDataBase->addItem(key, value);
    }
    return "OK";
}

std::string RedisHelper::setex(const std::string& key, const RedisValue& value) {
    auto currentNode = redisDataBase->searchItem(key);
    if (currentNode == nullptr) {
        return "key: " + key + " does not exists!";
    } else {
        // mark 这样会有并发性的问题吧，但其实没什么所谓，因为这里是单线程的。
        currentNode->value = value;
        // 改成了自己重载的函数
        //  redisDataBase->modifyItem(currentNode, value);
    }
    return "OK";
}

std::string RedisHelper::get(const std::string& key) {
    auto currentNode = redisDataBase->searchItem(key);
    if (currentNode == nullptr)
        return "key: " + key + " does not exist!";
    return currentNode->value.dump();
}

// 哈希表
//  HSET key field value：向哈希表中添加一个字段及其值。
//  HGET key field：获取哈希表中指定字段的值。
//  HDEL key field：删除哈希表 key 中的一个或多个指定字段。
//  HKEYS key：获取哈希表中的所有字段名。
//  HVALS key：获取哈希表中的所有值。

std::string RedisHelper::hset(const std::string& key, const std::vector<std::string>& filed) {
    std::string resMessage = "";
    auto currentNode = redisDataBase->searchItem(key);
    int count=0;
    auto addItem = [&filed](RedisValue::object& valueMap)->int{
        int count=0;
        for(int i=0;i+1<filed.size();i+=2){
            if(valueMap.count(filed[i]))continue;
            valueMap[filed[i]]=filed[i+1];   //！！！隐式转换，所以RedisValue类型的指针实际指向了RedisString类型。
            count++;
        }
        return count;
    };
    if (currentNode == nullptr) {
        RedisValue::object data;
        RedisValue redisHash(data);
        RedisValue::object& valueMap = redisHash.objectItems();
        count = addItem(valueMap);
        redisDataBase->addItem(key,valueMap);
    } else {
        if(currentNode->value.type()!=RedisValue::OBJECT){
            resMessage="The key:" +key+" "+"already exists and the value is not a hashtable!";
            return resMessage;
        }
        RedisValue::object& valueMap = currentNode->value.objectItems();
        count = addItem(valueMap);
    }
    resMessage="(integer) "+std::to_string(count);
    return resMessage;
}

std::string RedisHelper::hget(const std::string&key,const std::string&filed) {
    std::string resMessage = "";
    auto currentNode = redisDataBase->searchItem(key);
    if(currentNode==nullptr || currentNode->value.type()!=RedisValue::OBJECT){
        resMessage="(nil)";
    }else{
        RedisValue::object valueMap = currentNode->value.objectItems();
        if(!valueMap.count(filed)){
            resMessage="(nil)";
        }else{
            resMessage=valueMap[filed].stringValue();
        }
    }
    return resMessage;
}