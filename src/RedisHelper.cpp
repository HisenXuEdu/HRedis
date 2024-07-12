#include "RedisHelper.h"
#include "FileCreator.h"

void RedisHelper::flush(){
    // 打开文件并覆盖写入
    std::string filePath=getFilePath();
    std::ofstream outputFile(filePath);
    //检查文件是否打开成功
    if(!outputFile){
        std::cout<<"文件："<<filePath<<"打开失败"<<std::endl;
        return ;
    }
    auto currentNode = redisDataBase->gethead();
    while(currentNode!=nullptr){  //写入磁盘的是key和value，而不是节点结构。
        std::string key = currentNode->key;
        RedisValue value = currentNode->value;
        if(!key.empty()) //??? 这里不懂为什么要判断是不是empty。
            outputFile<<key<<":"<<value.dump()<<std::endl;
        currentNode = currentNode->forward[0];
    }
    outputFile.close();
}

std::string RedisHelper::getFilePath(){
    std::string folder = DEFAULT_DB_FOLDER;
    std::string fileName = DATABASE_FILE_NAME;
    std::string filePath=folder+"/"+fileName+dataBaseIndex; //文件路径
    return filePath;
}

void RedisHelper::loadData(std::string loadPath){
    redisDataBase->loadFile(loadPath);
}

std::string RedisHelper::select(int index){
    if(index<0||index>15){
        return "database index out of range.";
    }
    flush();
    redisDataBase=std::make_shared<SkipList<std::string, RedisValue>>();
    dataBaseIndex=std::to_string(index);
    std::string filePath = getFilePath();

    loadData(filePath);
    return "OK";
}

RedisHelper::RedisHelper(){
    redisDataBase = std::make_shared<SkipList<std::string, RedisValue>>();
    //每次检查并创建磁盘db
    FileCreator::createFolderAndFiles(DEFAULT_DB_FOLDER,DATABASE_FILE_NAME,DATABASE_FILE_NUMBER);
    std::string filePath=getFilePath();
    loadData(filePath);
};
RedisHelper::~RedisHelper(){};


//在声明总写了默认值，这里就不用写了。
std::string RedisHelper::set(const std::string& key, const RedisValue& value,const SET_MODEL model){
    if(model==XX){
        return setex(key,value);
    }else if(model==NX){
        return setnx(key,value);
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
    //todo 这里的实现结构并不是很好，因为每次都要调用很多遍searchItem
    auto currentNode=redisDataBase->searchItem(key);
    if(currentNode!=nullptr){
        return "key: "+ key +"  exists!";
    }else{
        redisDataBase->addItem(key,value);
    }
    return "OK";
}

std::string RedisHelper::setex(const std::string& key, const RedisValue& value){
    auto currentNode=redisDataBase->searchItem(key);
    if(currentNode==nullptr){
        return "key: "+ key +" does not exists!";
    }else{
        //mark 这样会有并发性的问题吧，但其实没什么所谓，因为这里是单线程的。
        currentNode->value=value;
        //改成了自己重载的函数
        // redisDataBase->modifyItem(currentNode, value);
    }
    return "OK";
}

std::string RedisHelper::get(const std::string& key){
    auto currentNode=redisDataBase->searchItem(key);
    if(currentNode==nullptr) return "key: "+ key +" does not exist!";
    return currentNode->value.dump();
}
