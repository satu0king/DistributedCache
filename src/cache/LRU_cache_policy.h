#include <list>
#include <map>

#include "cache_policy_interface.h"
#include "types.h"

class LRUCachePolicy : public CachePolicyInterface {
    std::map<key_pair_t, int> keyValueMap;
    std::map<key_pair_t, std::list<key_pair_t>::iterator> keyIteratorMap;
    std::list<key_pair_t> LRU_Queue;
    int size;

   public:
    bool hasEntry(std::string container, int key);
    int getEntry(std::string container, int key);
    void insert(std::string container, int key, int value);
    void erase(std::string container, int key);
    void allocateSize(int size);
    void reset();
    int getSize() { return size;}
    LRUCachePolicy(int size = 0) : size(size) {}
};