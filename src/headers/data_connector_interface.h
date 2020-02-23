#pragma once
#include <string>

class DataConnectorInterface {
   public:
    virtual int get(std::string container, int key) = 0;
    virtual void put(std::string container, int key, int value) = 0;
    virtual void createContainer(std::string container) = 0;
    virtual void erase(std::string container, int key) = 0;
    virtual void reset() = 0;
    virtual ~DataConnectorInterface(){};
};