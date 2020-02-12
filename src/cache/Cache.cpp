#include "cache/cache.h"

bool Cache::hasEntry(int key) { return cache->hasEntry(key); }
int Cache::getEntry(int key) {
    if (hasEntry(key)) return cache->getEntry(key);
    int value = DB->get(key);
    if (~value) cache->insert(key, value);
    return value;
}
void Cache::erase(int key) { cache->erase(key); }
void Cache::reset() { cache->reset(); }
void Cache::insert(int key, int value) {
    cache->insert(key, value);
    DB->put(key, value);
}

Cache::~Cache() {
    delete cache;
    delete DB;
}
