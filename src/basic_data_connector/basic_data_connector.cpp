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

#define TIMECHECK(x, message, limit)                                           \
                                                                               \
    auto start_##message = std::chrono::high_resolution_clock::now();          \
    x;                                                                         \
    auto finish_##message = std::chrono::high_resolution_clock::now();         \
    auto time_##message =                                                      \
        std::chrono::duration_cast<std::chrono::milliseconds>(                 \
            finish_##message - start_##message)                                \
            .count();                                                          \
    if (time_##message > limit)                                                \
        std::cout << "Time taken to run " << #message << " " << time_##message \
                  << std::endl;

int BasicDataConnector::getConnection() {
    int IP = inet_addr(ip.c_str());  // INADDR_ANY;
    struct sockaddr_in server, client;
    int sd = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = IP;
    server.sin_port = htons(port);

    TIMECHECK(
        int c = connect(sd, (struct sockaddr *)&server, sizeof(server)) < 0,
        CONNECTION_GET, 0);

    if (c < 0) {
        perror("connect()");
        throw std::runtime_error("Connection Failed to IP address - " + ip +
                                 ":" + std::to_string(port));
    }

    return sd;
}

int BasicDataConnector::get(std::string container, int key) {
    TIMECHECK(int sd = getConnection(), CONNECTION_GET_OVERALL, 0);
    int value = 0;
    TIMECHECK(
        {
            RequestType type = RequestType::GET;
            loop_write(sd, &type, sizeof(type));
            GetRequest request = {.key = key};
            strcpy(request.container, container.c_str());
            loop_write(sd, &request, sizeof(request));

            GetResponse response;
            loop_read(sd, &response, sizeof(response));
            close(sd);
            value = response.value;
        },
        GET, 55)
    return value;
}

void BasicDataConnector::reset() {
    int sd = getConnection();
    RequestType type = RequestType::RESET;
    loop_write(sd, &type, sizeof(type));
    close(sd);
}

void BasicDataConnector::put(std::string container, int key, int value) {
    int sd = getConnection();
    RequestType type = RequestType::PUT;
    loop_write(sd, &type, sizeof(type));
    PutRequest request = {.key = key, .value = value};
    strcpy(request.container, container.c_str());
    loop_write(sd, &request, sizeof(request));
    close(sd);
}

void BasicDataConnector::erase(std::string container, int key) {
    int sd = getConnection();
    RequestType type = RequestType::ERASE;
    loop_write(sd, &type, sizeof(type));
    EraseRequest request = {.key = key};
    strcpy(request.container, container.c_str());
    loop_write(sd, &request, sizeof(request));
    close(sd);
}

void BasicDataConnector::createContainer(std::string container) {
    int sd = getConnection();
    RequestType type = RequestType::CREATECONTAINER;
    loop_write(sd, &type, sizeof(type));
    EraseRequest request;
    strcpy(request.container, container.c_str());
    loop_write(sd, &request, sizeof(request));
    close(sd);
}