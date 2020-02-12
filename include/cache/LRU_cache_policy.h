#include <list>
#include <unordered_map>

#include "cache/cache_policy_interface.h"

class LRUCachePolicy : public CachePolicyInterface {
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
    void reset();
    LRUCachePolicy(int size = 0) : size(size) {}
};