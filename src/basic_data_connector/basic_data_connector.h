#pragma once
#include <string>

#include "data_connector_interface.h"

class BasicDataConnector : public DataConnectorInterface {
    std::string ip;
    int port;
    int getConnection();

   public:
    BasicDataConnector(std::string ip, int port) : ip(ip), port(port) {}
    int get(std::string container, int key);
    void put(std::string container, int key, int value);
    void erase(std::string container, int key);
    void createContainer(std::string container);
    void reset();
};