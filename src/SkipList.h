#ifndef SKIPLIST_H
#define SKIPLIST_H
#include<vector>
#include<memory>
#include<random>
#include<mutex>
#include "global.h"
// #include"RedisValue/RedisValue.h"

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
public:
    SkipList();
    ~SkipList();
    int randomLevel();
    bool addItem(const Key& key, const Value& value); //添加节点
    bool modifyItem(const Key& key, const Value& value); //修改节点
    std::shared_ptr<SkipListNode<Key,Value>> searchItem(const Key& key); //查找节点
    bool deleteItem(const Key& key); //删除节点
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
SkipList<Key,Value>::~SkipList(){
    
}

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