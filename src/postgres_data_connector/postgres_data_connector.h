#pragma once

#include <pqxx/pqxx>
#include <string>

#include "data_connector_interface.h"

class PostgresDataConnector : public DataConnectorInterface {
    pqxx::connection *c;
    std::string ip;
    int port;
    std::string username;
    std::string password;
    std::string database;
    void getConnection();

   public:
    PostgresDataConnector(std::string database,
                          std::string username = "postgres",
                          std::string password = "postgres",
                          std::string ip = "127.0.0.1", int port = 5432)
        : ip(ip),
          port(port),
          username(username),
          database(database),
          password(password) {
        getConnection();
    }
    int get(std::string container, int key);
    void put(std::string container, int key, int value);
    void erase(std::string container, int key);
    void createContainer(std::string container);
    void reset();
    ~PostgresDataConnector();
};

/*

create database mock_database;

CREATE TABLE CONTAINER2(
   KEY INT PRIMARY KEY     NOT NULL,
   VALUE           INT    NOT NULL
);

*/