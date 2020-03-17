#pragma once

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

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
    CREATECONTAINER,
    GOSSIP
};

struct Address {
    std::string ip;
    int port;
    Address(const std::string ip, const int port) : ip(ip), port(port){};
    bool operator==(const Address& other) {
        return ip == other.ip && port == other.port;
    }
    bool operator<(const Address& other) const {
        if (ip != other.ip) return ip < other.ip;
        return port < other.port;
    }
    std::string toString() const {
        return ip + ":" + std::to_string(port);
    }
};



/* A simple routine calling UNIX write() in a loop */
static ssize_t loop_write(int fd, const void* data, size_t size) {
    ssize_t ret = 0;
    while (size > 0) {
        ssize_t r;
        if ((r = write(fd, data, size)) < 0) return r;
        if (r == 0) break;
        ret += r;
        data = (const char*)data + r;
        size -= (size_t)r;
    }
    return ret;
}


/* A simple routine calling UNIX read() in a loop */
static ssize_t loop_read(int fd, void* data, size_t size) {
    ssize_t ret = 0;
    while (size > 0) {
        ssize_t r;
        if ((r = read(fd, data, size)) < 0) return r;
        if (r == 0) break;
        ret += r;
        data = (char*)data + r;
        size -= (size_t)r;
    }
    return ret;
}