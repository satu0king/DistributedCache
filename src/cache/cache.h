#pragma once 

#include "cache_policy_interface.h"
#include "data_connector_interface.h"

class Cache {
    CachePolicyInterface *cache;
    DataConnectorInterface *DB;

   public:
    Cache(CachePolicyInterface *cache, DataConnectorInterface *DB) : cache(cache), DB(DB) {}
    bool hasEntry(int key);
    int getEntry(int key);
    void erase(int key);
    void reset();
    void insert(int key, int value);
    ~Cache();
};