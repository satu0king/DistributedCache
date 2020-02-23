#include "basic_data_connector.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <iterator>

#include "requests.h"

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

int BasicDataConnector::get(std::string container, int key) {
    int sd = getConnection();
    RequestType type = RequestType::GET;
    write(sd, &type, sizeof(type));
    GetRequest request = {.key = key};
    strcpy(request.container, container.c_str());
    write(sd, &request, sizeof(request));

    GetResponse response;
    read(sd, &response, sizeof(response));
    close(sd);
    return response.value;
}

void BasicDataConnector::reset() {
    int sd = getConnection();
    RequestType type = RequestType::RESET;
    write(sd, &type, sizeof(type));
    close(sd);
}

void BasicDataConnector::put(std::string container, int key, int value) {
    int sd = getConnection();
    RequestType type = RequestType::PUT;
    write(sd, &type, sizeof(type));
    PutRequest request = {.key = key, .value = value};
    strcpy(request.container, container.c_str());
    write(sd, &request, sizeof(request));
    close(sd);
}

void BasicDataConnector::erase(std::string container, int key) {
    int sd = getConnection();
    RequestType type = RequestType::ERASE;
    write(sd, &type, sizeof(type));
    EraseRequest request = {.key = key};
    strcpy(request.container, container.c_str());
    write(sd, &request, sizeof(request));
    close(sd);
}

void BasicDataConnector::createContainer(std::string container) {
     int sd = getConnection();
    RequestType type = RequestType::CREATECONTAINER;
    write(sd, &type, sizeof(type));
    EraseRequest request;
    strcpy(request.container, container.c_str());
    write(sd, &request, sizeof(request));
    close(sd);
}