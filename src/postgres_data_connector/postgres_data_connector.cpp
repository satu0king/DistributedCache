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

int PostgresDataConnector::get(int key) {
    pqxx::work txn(*c);

    usleep(50 * 1000);

    try {
        pqxx::row r = txn.exec1(
            "SELECT VALUE "
            "FROM CONTAINER1 "
            "WHERE key =" +
            txn.quote(key));
        txn.commit();
        int value = r[0].as<int>();
        return value;
    } catch (const pqxx::unexpected_rows& exception) {
        txn.commit();
        return -1;
    }
}

void PostgresDataConnector::reset() {
    pqxx::work txn(*c);
    txn.exec0("delete from CONTAINER1");
    txn.commit();
}

void PostgresDataConnector::put(int key, int value) {
    pqxx::work txn(*c);

    txn.exec0(
        "INSERT INTO CONTAINER1 (key, value) VALUES (" + txn.quote(key) + ", " +
        txn.quote(value) +
        ") ON CONFLICT (key) DO UPDATE SET value = " + txn.quote(value)

    );
    txn.commit();
}

void PostgresDataConnector::erase(int key) {
    pqxx::work txn(*c);
    txn.exec0("delete from CONTAINER1 where key = " + txn.quote(key));
    txn.commit();
}

PostgresDataConnector::~PostgresDataConnector() {
    delete c;
}
