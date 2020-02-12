#pragma once
#include "database_connector_interface.h"

class MockDatabaseConnector : public DatabaseConnectorInterface {
   public:
    virtual bool get(int key);
    virtual void put(int key, int value);
    virtual void erase(int key);
};