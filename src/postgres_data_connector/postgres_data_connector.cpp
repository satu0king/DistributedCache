#include "postgres_data_connector.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <iterator>

#include "requests.h"

void PostgresDataConnector::getConnection() {
    char connection[200];
    sprintf(connection, "postgresql://%s:%s@%s:%d/%s", username.c_str(),
            password.c_str(), ip.c_str(), port, database.c_str());
    std::cout << connection << std::endl;
    c = new pqxx::connection(connection);
}

int PostgresDataConnector::get(std::string container, int key) {
    pqxx::work txn(*c);

    usleep(50 * 1000);

    try {
        pqxx::row r = txn.exec1(
            "SELECT VALUE "
            "FROM " +
            container + " WHERE key =" + txn.quote(key));
        txn.commit();
        int value = r[0].as<int>();
        return value;
    } catch (const pqxx::unexpected_rows& exception) {
        txn.commit();
        return -1;
    }
}

void PostgresDataConnector::reset() {
    // pqxx::work txn(*c);
    // txn.exec0("delete from CONTAINER1");
    // txn.commit();
    throw std::runtime_error("reset() is unsupported");
}

void PostgresDataConnector::createContainer(std::string container) {
    pqxx::work txn(*c);
    txn.exec("CREATE TABLE " + container +
             " (KEY INT PRIMARY KEY NOT NULL, VALUE INT NOT "
             "NULL) ");
    txn.commit();
}

void PostgresDataConnector::put(std::string container, int key, int value) {
    pqxx::work txn(*c);

    txn.exec0("INSERT INTO " + container + " (key, value) VALUES (" +
              txn.quote(key) + ", " + txn.quote(value) +
              ") ON CONFLICT (key) DO UPDATE SET value = " + txn.quote(value)

    );
    txn.commit();
}

void PostgresDataConnector::erase(std::string container, int key) {
    pqxx::work txn(*c);
    txn.exec0("delete from " + container + " where key = " + txn.quote(key));
    txn.commit();
}

PostgresDataConnector::~PostgresDataConnector() { delete c; }
