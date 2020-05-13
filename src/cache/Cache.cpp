#include "cache.h"

#include <iostream>

#define TIMECHECK(x, message, limit)                                           \
                                                                               \
    auto start_##message = std::chrono::high_resolution_clock::now();          \
    x;                                                                         \
    auto finish_##message = std::chrono::high_resolution_clock::now();         \
    auto time_##message =                                                      \
        std::chrono::duration_cast<std::chrono::milliseconds>(                 \
            finish_##message - start_##message)                                \
            .count();                                                          \
    if (time_##message > limit)                                                \
        std::cout << "Time taken to run " << #message << " " << time_##message \
                  << std::endl;

bool Cache::hasEntry(std::string container, int key) {
    return cache->hasEntry(container, key);
}
int Cache::getEntry(std::string container, int key) {
    {
        TIMECHECK(std::lock_guard<std::mutex> guard(cacheLock), lock_time, 1);
        if (hasEntry(container, key)) return cache->getEntry(container, key);
    }
    TIMECHECK(int value = DB->get(container, key), DB_GET, 55);
    if (~value) {
        TIMECHECK(std::lock_guard<std::mutex> guard(cacheLock), lock_time, 1);
        cache->insert(container, key, value);
    }
    return value;
}
void Cache::erase(std::string container, int key) {
    TIMECHECK(std::lock_guard<std::mutex> guard(cacheLock), lock_time, 1);
    cache->erase(container, key);
}
void Cache::reset() {
    TIMECHECK(std::lock_guard<std::mutex> guard(cacheLock), lock_time, 1);
    cache->reset();
}
void Cache::insert(std::string container, int key, int value) {
    {
        TIMECHECK(std::lock_guard<std::mutex> guard(cacheLock), lock_time, 1);
        cache->insert(container, key, value);
    }
    DB->put(container, key, value);
}

Cache::~Cache() {
    delete cache;
    delete DB;
}
