#pragma once
#include <string> 

class CacheConnector {
    std::string cacheIP;
    int cachePort;

    int getCacheConnection();

   public:
    CacheConnector(std::string cacheIP, int cachePort) : cacheIP(cacheIP), cachePort(cachePort) {}
    int get(int key);
    void put(int key, int value);
};