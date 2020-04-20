#pragma once

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <queue>
#include <string>
#include <vector>

#include "server_interface.h"

struct ThreadObject;
void* threadHandler(void* _pool);

class ServerThreadPool {
    std::queue<int> connectionQueue;
    pthread_mutex_t queueLock = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t c_cons = PTHREAD_COND_INITIALIZER;
    pthread_cond_t c_prod = PTHREAD_COND_INITIALIZER;
    int port;
    std::string ip;
    int threadCount;
    MultiThreadedServerInterface* server;
    std::vector<pthread_t> threads;
    int socketConnection;
    int queueCapacity;

   public:
    ServerThreadPool(MultiThreadedServerInterface* server, std::string ip,
                     int port, int threadCount = 5, int queueCapacity = 10)
        : ip(ip),
          port(port),
          threadCount(threadCount),
          server(server),
          threads(threadCount),
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
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < threadCount; i++) {
            if (pthread_create(&threads[i], NULL, threadHandler, this) < 0) {
                perror("pthread_create()");
                exit(EXIT_FAILURE);
            }
        }

        listen(socketConnection, 10);

        while (1) {
            int nsd =
                accept(socketConnection, (struct sockaddr*)&client, &clientLen);
            pthread_mutex_lock(&queueLock);

            while (connectionQueue.size() >= queueCapacity) {
                std::cout << "Queue Full";
                pthread_cond_wait(&c_prod, &queueLock);
            }

            connectionQueue.push(nsd);
            pthread_mutex_unlock(&queueLock);
            pthread_cond_signal(&c_cons);
        }
    }

    friend void* threadHandler(void* _pool);
};

void* threadHandler(void* _pool) {
    ServerThreadPool* pool = (ServerThreadPool*)_pool;
    MultiThreadedServerInterface* server = pool->server;

    while (1) {
        pthread_mutex_lock(&pool->queueLock);

        while (pool->connectionQueue.empty())
            pthread_cond_wait(
                &pool->c_cons,
                &pool->queueLock);  // Wait for new message to deliver

        int connection = pool->connectionQueue.front();
        pool->connectionQueue.pop();

        pthread_mutex_unlock(&pool->queueLock);
        pthread_cond_signal(&pool->c_prod);

        server->controller(connection);
        close(connection);
    }
}