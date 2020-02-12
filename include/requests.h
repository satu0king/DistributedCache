#pragma once

struct GetRequest {
    int key;
};

struct EraseRequest {
    int key;
};

struct GetResponse {
    int key;
    int value;
};

struct PutRequest {
    int key;
    int value;
};

enum RequestType {
    GET,
    PUT,
    ERASE
};