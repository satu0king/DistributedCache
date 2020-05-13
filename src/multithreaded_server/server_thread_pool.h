#pragma once

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include "server_interface.h"

struct ThreadObject;
void* threadHandler(void* _pool);

class ServerThreadPool {
    std::queue<int> connectionQueue;
    std::mutex queueLock;
    std::condition_variable c_cons;
    std::condition_variable c_prod;
    int port;
    std::string ip;
    int threadCount;
    MultiThreadedServerInterface* server;
    std::vector<std::thread> threads;
    int socketConnection;
    int queueCapacity;

   public:
    ServerThreadPool(MultiThreadedServerInterface* server, std::string ip,
                     int port, int threadCount = 30, int queueCapacity = 20)
        : ip(ip),
          port(port),
          threadCount(threadCount),
          server(server),
          queueCapacity(queueCapacity) {}

    void start() {
        socklen_t clientLen;

        struct sockaddr_in server, client;

        socketConnection = socket(AF_INET, SOCK_STREAM, 0);

        server.sin_family = AF_INET;
        server.sin_addr.s_addr = inet_addr(ip.c_str());
        server.sin_port = htons(port);

        {
            int optionValue = 1;
            setsockopt(socketConnection, SOL_SOCKET, SO_REUSEADDR, &optionValue,
                       sizeof(int));
        }

        if (bind(socketConnection, (struct sockaddr*)&server, sizeof(server)) <
            0) {
            perror("bind failed");
            throw std::runtime_error("bind failed");
        }

        for (int i = 0; i < threadCount; i++)
            threads.push_back(
                std::thread(&ServerThreadPool::threadHandler, this));

        listen(socketConnection, 30);

        while (1) {
            int nsd =
                accept(socketConnection, (struct sockaddr*)&client, &clientLen);

            std::unique_lock lock(queueLock);

            while (connectionQueue.size() >= queueCapacity) {
                std::cout << "Queue Full";
                c_prod.wait(lock);
            }

            if (connectionQueue.size() >= 5) {
                std::cout << "Queue Size: " << connectionQueue.size()
                          << std::endl;
            }

            connectionQueue.push(nsd);
            lock.unlock();
            c_cons.notify_one();
        }
    }

    void stop() {
        for (int i = 0; i < threadCount; i++) {
            // pthread_cancel(threads[i]);
        }
    }

    void threadHandler() {
        while (1) {
            std::unique_lock lock(queueLock);

            while (connectionQueue.empty()) c_cons.wait(lock);

            int connection = connectionQueue.front();
            connectionQueue.pop();

            lock.unlock();
            c_prod.notify_one();

            server->controller(connection);
            close(connection);
        }
    }
};
