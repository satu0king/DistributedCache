#pragma once
#include "database_connector_interface.h"
#include <string> 

class MockDatabaseConnector : public DatabaseConnectorInterface {
    std::string databaseIP;
    int databasePort;

    int getDBConnection();

   public:
    MockDatabaseConnector(std::string databaseIP, int databasePort) : databaseIP(databaseIP), databasePort(databasePort) {}
    int get(int key);
    void put(int key, int value);
    void erase(int key);
};