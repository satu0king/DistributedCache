#include "mock_database_connector.h"
#include "requests.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <iostream>
#include <iterator>
#include <unistd.h>

int MockDatabaseConnector::getDBConnection() {
    int IP = inet_addr(databaseIP.c_str());  // INADDR_ANY;
    int port = databasePort;
    struct sockaddr_in server, client;
    int sd = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = IP;
    server.sin_port = htons(5555);

    if (connect(sd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect()");
        exit(0);
    }

    return sd;
}


int MockDatabaseConnector::get(int key) {
    int sd = getDBConnection();
    RequestType type = GET;
    write(sd, &type, sizeof(type));
    GetRequest request = {.key = key};
    write(sd, &request, sizeof(request));

    GetResponse response;
    read(sd, &response, sizeof(response));
    close(sd);
    return response.value;
}

void MockDatabaseConnector::put(int key, int value) {
    int sd = getDBConnection();
    RequestType type = PUT;
    write(sd, &type, sizeof(type));
    PutRequest request = {.key = key, .value = value};
    write(sd, &request, sizeof(request));
    close(sd);
}

void MockDatabaseConnector::erase(int key) {
    int sd = getDBConnection();
    RequestType type = ERASE;
    write(sd, &type, sizeof(type));
    EraseRequest request = {.key = key};
    write(sd, &request, sizeof(request));
    close(sd);
}

