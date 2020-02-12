#include "cache/LRU_cache_policy.h"

void LRUCachePolicy::allocateSize(int size) {
    this->size = size;
}

bool LRUCachePolicy::hasEntry(int key) {
    return keyValueMap.count(key);
}

int LRUCachePolicy::getEntry(int key) {
    assert(hasEntry(key));

    LRU_Queue.erase(keyIteratorMap[key]);
    LRU_Queue.push_front(key);

    keyIteratorMap[key] = LRU_Queue.begin();

    return keyValueMap[key];
}

void LRUCachePolicy::erase(int key) {
    if (!hasEntry(key))
        return;

    keyValueMap.erase(key);
    LRU_Queue.erase(keyIteratorMap[key]);
    keyIteratorMap.erase(key);
}

void LRUCachePolicy::insert(int key, int value) {
    assert(size > 0);

    if (hasEntry(key)) {
        LRU_Queue.erase(keyIteratorMap[key]);
    } else if (size == keyValueMap.size()) {
        int evictedKey = LRU_Queue.back();
        LRU_Queue.pop_back();
        keyValueMap.erase(evictedKey);
        keyIteratorMap.erase(evictedKey);
    }

    LRU_Queue.push_front(key);
    keyIteratorMap[key] = LRU_Queue.begin();

    keyValueMap.erase(key);
    LRU_Queue.erase(keyIteratorMap[key]);
    keyIteratorMap.erase(key);

    keyValueMap[key] = value;
}
