#pragma once

class CachePolicyInterface{
    public:
    virtual bool hasEntry(int key) = 0;
    virtual int getEntry(int key) = 0;
    virtual void insert(int key, int value) = 0;
    virtual void erase(int key) = 0;
    virtual void allocateSize(int size) = 0;
};