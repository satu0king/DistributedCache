#pragma once
#include <string>

#include "data_connector_interface.h"

class BasicDataConnector : public DataConnectorInterface {
    std::string ip;
    int port;
    int getConnection();

   public:
    BasicDataConnector(std::string ip, int port)
        : ip(ip), port(port) {}
    int get(int key);
    void put(int key, int value);
    void erase(int key);
    void reset();
};