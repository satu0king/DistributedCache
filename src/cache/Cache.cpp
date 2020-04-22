#include "cache.h"

bool Cache::hasEntry(std::string container, int key) {
    return cache->hasEntry(container, key);
}
int Cache::getEntry(std::string container, int key) {
    {
        std::lock_guard<std::mutex> guard(cacheLock);
        if (hasEntry(container, key)) return cache->getEntry(container, key);
    }
    int value = DB->get(container, key);
    if (~value) {
        std::lock_guard<std::mutex> guard(cacheLock);
        cache->insert(container, key, value);
    }
    return value;
}
void Cache::erase(std::string container, int key) {
    std::lock_guard<std::mutex> guard(cacheLock);
    cache->erase(container, key);
}
void Cache::reset() {
    std::lock_guard<std::mutex> guard(cacheLock);
    cache->reset();
}
void Cache::insert(std::string container, int key, int value) {
    std::lock_guard<std::mutex> guard(cacheLock);
    cache->insert(container, key, value);
    DB->put(container, key, value);
}

Cache::~Cache() {
    delete cache;
    delete DB;
}
