#pragma once

#include <vector>

struct Buffer {
    std::vector<char> buffer;
    char& operator[](int i) { return buffer[i]; }
    Buffer(int size = 0) : buffer(size){};
    Buffer(void* data, int size) : buffer(size) {
        memcpy(buffer.data(), data, size);
    };
    int size() const { return buffer.size(); }
    void addBuffer(Buffer& other) {
        buffer.insert(buffer.end(), other.buffer.begin(), other.buffer.end());
    }
    void addBuffer(Buffer&& other) {
        buffer.insert(buffer.end(), other.buffer.begin(), other.buffer.end());
    }
    void addBuffer(void* data, int size) {
        buffer.insert(buffer.end(), (char*)data, (char*)data + size);
    }
    char* data() { return buffer.data(); }
};