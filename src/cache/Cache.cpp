#include "cache/cache_policy_interface.h"
#include "database_connector_interface.h"

class CacheNode{
    CachePolicyInterface *cache;
    DatabaseConnectorInterface *DB;
    public:
    CacheNode(CachePolicyInterface *cache): cache(cache) {}
    bool hasEntry(int key) {return cache->hasEntry(key);} 
    int getEntry(int key)  {
        if(hasEntry(key)) return cache->getEntry(key);
        int value = DB->get(key);
        if(~value)
            cache->insert(key, value);
        return value;
    }
    void erase(int key) {cache->erase(key);}

};