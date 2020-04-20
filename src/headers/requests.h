#pragma once

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>

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

enum class RequestType { GET, PUT, ERASE, RESET, CREATECONTAINER, GOSSIP, COORDINATE_GET, COORDINATE_PUT };

struct Address {
    std::string ip;
    int port;
    Address(const std::string ip = "invalid", const int port = -1) : ip(ip), port(port){};
    bool operator==(const Address& other) {
        return ip == other.ip && port == other.port;
    }
    bool operator<(const Address& other) const {
        if (ip != other.ip) return ip < other.ip;
        return port < other.port;
    }
    std::string toString() const { return ip + ":" + std::to_string(port); }

    static int getConnection(Address addr) {
        int IP = inet_addr(addr.ip.c_str());  // INADDR_ANY;
        struct sockaddr_in server;
        int sd = socket(AF_INET, SOCK_STREAM, 0);
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = IP;
        server.sin_port = htons(addr.port);

        if (connect(sd, (struct sockaddr*)&server, sizeof(server)) < 0) {
            perror("connect failed: ");
            return -1;
        }

        return sd;
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

/*

Hierarchy Structure. Each PayloadHeader is for a single artifact

    GossipPayload
        PayloadHeader
            Buffer
        PayloadHeader
            Buffer
*/

struct GossipPayload {
    int gossipArtifactCount;
};

struct PayloadHeader {
    char type[20];
    int size;  // (not including header size)
};