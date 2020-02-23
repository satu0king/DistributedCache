#pragma once

struct GetRequest {
    char container[10];
    int key;
};


struct CreateRequest {
    char container[10];
};

struct EraseRequest {
    char container[10];
    int key;
};

struct GetResponse {
    int value;
};

struct PutRequest {
    char container[10];
    int key;
    int value;
};

enum class RequestType {
    GET,
    PUT,
    ERASE,
    RESET,
    CREATECONTAINER
};