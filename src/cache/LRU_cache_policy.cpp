#include "LRU_cache_policy.h"

void LRUCachePolicy::allocateSize(int size) { this->size = size; }

bool LRUCachePolicy::hasEntry(std::string container, int key) {
    return keyValueMap.count({container, key});
}

int LRUCachePolicy::getEntry(std::string container, int key) {
    auto key_pair = make_pair(container, key);
    assert(hasEntry(container, key));

    LRU_Queue.erase(keyIteratorMap[key_pair]);
    LRU_Queue.push_front(key_pair);

    keyIteratorMap[key_pair] = LRU_Queue.begin();

    return keyValueMap[key_pair];
}

void LRUCachePolicy::erase(std::string container, int key) {
    if (!hasEntry(container, key)) return;

    auto key_pair = make_pair(container, key);
    keyValueMap.erase(key_pair);
    LRU_Queue.erase(keyIteratorMap[key_pair]);
    keyIteratorMap.erase(key_pair);
}

void LRUCachePolicy::insert(std::string container, int key, int value) {
    assert(size > 0);

    auto key_pair = make_pair(container, key);
    if (hasEntry(container, key)) {
        LRU_Queue.erase(keyIteratorMap[key_pair]);
    } else if (size == keyValueMap.size()) {
        auto evictedKey = LRU_Queue.back();
        LRU_Queue.pop_back();
        keyValueMap.erase(evictedKey);
        keyIteratorMap.erase(evictedKey);
    }

    LRU_Queue.push_front(key_pair);
    keyIteratorMap[key_pair] = LRU_Queue.begin();
    keyValueMap[key_pair] = value;
}
void LRUCachePolicy::reset() {
    LRU_Queue.clear();
    keyValueMap.clear();
    keyIteratorMap.clear();
}
