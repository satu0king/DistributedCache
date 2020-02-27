#pragma once
#include <string>

class CachePolicyInterface {
   public:
    virtual bool hasEntry(std::string container, int key) = 0;
    virtual int getEntry(std::string container, int key) = 0;
    virtual void insert(std::string container, int key, int value) = 0;
    virtual void erase(std::string container, int key) = 0;
    virtual void reset() = 0;
    virtual void allocateSize(int size) = 0;
    virtual int getSize() = 0;
    virtual ~CachePolicyInterface(){};
};