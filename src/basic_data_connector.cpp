#include "basic_data_connector.h"
#include "requests.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <iostream>
#include <iterator>
#include <unistd.h>

int BasicDataConnector::getConnection() {
    int IP = inet_addr(ip.c_str());  // INADDR_ANY;
    struct sockaddr_in server, client;
    int sd = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = IP;
    server.sin_port = htons(port);

    if (connect(sd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect()");
        exit(0);
    }

    return sd;
}


int BasicDataConnector::get(int key) {
    int sd = getConnection();
    RequestType type = GET;
    write(sd, &type, sizeof(type));
    GetRequest request = {.key = key};
    write(sd, &request, sizeof(request));

    GetResponse response;
    read(sd, &response, sizeof(response));
    close(sd);
    return response.value;
}

void BasicDataConnector::reset() {
    int sd = getConnection();
    RequestType type = RESET;
    write(sd, &type, sizeof(type));
    close(sd);
}

void BasicDataConnector::put(int key, int value) {
    int sd = getConnection();
    RequestType type = PUT;
    write(sd, &type, sizeof(type));
    PutRequest request = {.key = key, .value = value};
    write(sd, &request, sizeof(request));
    close(sd);
}

void BasicDataConnector::erase(int key) {
    int sd = getConnection();
    RequestType type = ERASE;
    write(sd, &type, sizeof(type));
    EraseRequest request = {.key = key};
    write(sd, &request, sizeof(request));
    close(sd);
}

