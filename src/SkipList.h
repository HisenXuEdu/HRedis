#ifndef SKIPLIST_H
#define SKIPLIST_H
#include<vector>
#include<memory>
#include<random>
#include<mutex>
#include "global.h"
#define  DELIMITER ":"
#include"RedisValue/RedisValue.h"

#define  PROBABILITY_FACTOR 0.25  //这里就相当与每层是下一层节点的factor倍
#define MAX_SKIP_LIST_LEVEL 32

template<typename Key,typename Value>
class SkipListNode{
public:
    Key key;
    Value value;
    std::vector<std::shared_ptr<SkipListNode<Key,Value>>> forward;
    SkipListNode(Key key, Value value, int maxLevel=MAX_SKIP_LIST_LEVEL):
    key(key),value(value),forward(maxLevel,nullptr){};
};

template<typename Key, typename Value>
class SkipList{
private:
    int currentLevel; //当前跳表的最大层数
    std::shared_ptr<SkipListNode<Key,Value>> head; //头节点
    std::mt19937 generator{ std::random_device{}()}; //随机数生成器    //??? 随机数生成这里不太懂。
    std::uniform_real_distribution<double> distribution; //随机数分布
    int elementNumber=0;
    std::mutex mutex;
    std::ifstream readFile; //读文件
public:
    SkipList();
    ~SkipList();
    int randomLevel();
    bool addItem(const Key& key, const Value& value); //添加节点
    bool modifyItem(const Key& key, const Value& value); //修改节点
    //mark 自己重载的通过传入节点和value修改
    // bool modifyItem(std::shared_ptr<SkipList<Key,Value>>& currentNode, const Value& value); //修改节点
    std::shared_ptr<SkipListNode<Key,Value>> searchItem(const Key& key); //查找节点
    bool deleteItem(const Key& key); //删除节点
    void loadFile(std::string load_path);
    bool parseString(const std::string&line,std::string&key,std::string&value);
    bool isVaildString(const std::string&line);
    std::shared_ptr<SkipListNode<Key,Value>> gethead(){return head;};
};

/*--------------函数定义---------------------*/
template<typename Key,typename Value>
SkipList<Key,Value>::SkipList()
    :currentLevel(0),distribution(0, 1)
{
    Key key;
    Value value;
    this->head=std::make_shared<SkipListNode<Key,Value>>(key,value); //初始化头节点,层数为最大层数
}

template<typename Key,typename Value>
SkipList<Key,Value>::~SkipList(){}

//随机生成新节点的层数
template<typename Key, typename Value>
int SkipList<Key,Value>::randomLevel()
{
    int level=1;
    while(distribution(generator)< PROBABILITY_FACTOR
        && level<MAX_SKIP_LIST_LEVEL){
        level++;
    }
    return level;
}

template<typename Key,typename Value>
void SkipList<Key,Value>::loadFile(std::string load_path){
    std::cout<<"SkipList::loadFile NOT YET"<<std::endl;
    
    readFile.open(load_path); //打开文件
    if(!readFile.is_open()){ 
        mutex.unlock();
        return;
    }
    std::string line;
    std::string key;
    std::string value;
    std::string err;
    while(std::getline(readFile,line)){
        if(parseString(line,key,value)){
            //todo 要实现文件中string写入跳表。
            //??? 开源代码中这个addItem并没有实现去重，在RedisHelper::set中实现了去重，但是这样会导致如果改了db文件加入重复key，会导致重复。
            auto redisValue=RedisValue::parse(value,err);
            if(err=="fail") continue;
            addItem(key,redisValue);
            std::cout<<"parseString OK"<<std::endl;
        }
    }
}

template<typename Key,typename Value>
bool SkipList<Key,Value>::isVaildString(const std::string&line){
    if(line.empty()) return false;
    if(line.find(DELIMITER)==std::string::npos) return false;
    return true;
}


template<typename Key,typename Value>
bool SkipList<Key,Value>::parseString(const std::string&line,std::string&key,std::string&value){
    if(!isVaildString(line)){
        return false;
    }
    int index=line.find(DELIMITER); //找到分隔符的位置 返回的是分隔符的位置
    key=line.substr(0,index);
    value=line.substr(index+1);
    return true;
}

template<typename Key,typename Value>
bool SkipList<Key,Value>::addItem(const Key& key, const Value& value){
    mutex.lock();
    auto currentNode=this->head;
    std::vector<std::shared_ptr<SkipListNode<Key,Value>>> update(MAX_SKIP_LIST_LEVEL,head);
    for(int i=currentLevel-1;i>=0;i--){
        while(currentNode->forward[i]&&currentNode->forward[i]->key<key){
            currentNode=currentNode->forward[i];
        }
        update[i]=currentNode;
    }
    int newLevel = this->randomLevel();
    currentLevel = std::max(newLevel,currentLevel);
    std::shared_ptr<SkipListNode<Key,Value>> newNode=std::make_shared<SkipListNode<Key,Value>>(key,value,newLevel);
    for(int i=0;i<newLevel;i++){
        newNode->forward[i] = update[i]->forward[i];
        update[i]->forward[i]=newNode;
    }
    elementNumber++;
    mutex.unlock();
    return true;
}

template<typename Key,typename Value>
bool SkipList<Key,Value>::modifyItem(const Key& key, const Value& value){
    auto currentNode = searchItem(key);
    mutex.lock();
    if(currentNode==nullptr) {
        mutex.unlock();
        return false;
    }
    currentNode->value = value;
    mutex.unlock();
    return true;
}

//??? 这里智能指针模板推导不好用？
// template<typename Key,typename Value>
// bool SkipList<Key,Value>::modifyItem(std::shared_ptr<SkipList<Key,Value>>& currentNode, const Value& value){
//     mutex.lock();
//     currentNode->value=value;
//     mutex.unlock();
//     return true;
// }

template<typename Key,typename Value>
std::shared_ptr<SkipListNode<Key,Value>> SkipList<Key,Value>::searchItem(const Key& key){
    //??? 别人的实现中这里也加了锁，但是我觉得没必要。
    auto currentNode=this->head;
    if(!currentNode) return nullptr;
    // std::vector<std::shared_ptr<SkipListNode<Key,Value>>> search(MAX_SKIP_LIST_LEVEL,nullptr);
    for(int i=currentLevel-1;i>=0;i--){
        while(currentNode->forward[i]&&currentNode->forward[i]->key<key){
            currentNode=currentNode->forward[i];
        }
    }
    currentNode=currentNode->forward[0];
    if(currentNode&&currentNode->key==key){
        return currentNode;
    }
    return nullptr;
}


#endif