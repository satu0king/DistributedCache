#include "cache.h"

bool Cache::hasEntry(std::string container, int key) {
    return cache->hasEntry(container, key);
}
int Cache::getEntry(std::string container, int key) {
    if (hasEntry(container, key)) return cache->getEntry(container, key);
    int value = DB->get(container, key);
    if (~value) cache->insert(container, key, value);
    return value;
}
void Cache::erase(std::string container, int key) {
    cache->erase(container, key);
}
void Cache::reset() { cache->reset(); }
void Cache::insert(std::string container, int key, int value) {
    cache->insert(container, key, value);
    DB->put(container, key, value);
}

Cache::~Cache() {
    if (int i = 0; i < 10) {
    }
    delete cache;
    delete DB;
}
