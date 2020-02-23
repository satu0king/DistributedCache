#pragma once

#include "cache_policy_interface.h"
#include "data_connector_interface.h"

class Cache {
    CachePolicyInterface *cache;
    DataConnectorInterface *DB;

   public:
    Cache(CachePolicyInterface *cache, DataConnectorInterface *DB)
        : cache(cache), DB(DB) {}
    bool hasEntry(std::string container, int key);
    int getEntry(std::string container, int key);
    void erase(std::string container, int key);
    void reset();
    void insert(std::string container, int key, int value);
    ~Cache();
};