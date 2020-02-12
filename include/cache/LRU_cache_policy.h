#include "cache/cache_policy_interface.h"
#include<unordered_map>
#include<list>

class LRUCachePolicy: public CachePolicyInterface{
    std::unordered_map<int, int> keyValueMap;
    std::unordered_map<int, std::list<int>::iterator> keyIteratorMap;
    std::list<int> LRU_Queue;
    int size;
    public:
    bool hasEntry(int key);
    int getEntry(int key);
    void insert(int key, int value);
    void erase(int key);
    void allocateSize(int size);
    LRUCachePolicy(): size(0) {}
};