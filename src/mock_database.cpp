// Copyright Vladimir Prus 2002-2004.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

/* The simplest usage of the library.
 */

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <unordered_map>
#include "requests.h"

std::unordered_map<int, int> db;
int sd;

void killServer(int ret = EXIT_SUCCESS) {
    std::cout << "Shutting Down Server ...\n";
    close(sd);
    exit(ret);
}

void handle_my(int sig) {
    switch (sig) {
        case SIGINT:
            killServer();
            break;
    }
}

po::variables_map config;

void initConfig(int ac, char *av[]) {
    try {
        po::options_description desc("Allowed options");
        auto init = desc.add_options();
        init("help", "produce help message");
        init("size,s", po::value<int>()->default_value(0), "Initial seed size of the database");
        init("key-range,r", po::value<int>()->default_value(1 << 20), "Maximum Key Value");
        init("response-delay,d", po::value<int>()->default_value(500), "Reequest Delay in milliseconds");

        po::store(po::parse_command_line(ac, av, desc), config);
        po::notify(config);

        if (config.count("help")) {
            std::cout << desc << "\n";
            killServer();
        }
    } catch (std::exception &e) {
        std::cerr << "error: " << e.what() << "\n";
        killServer(EXIT_FAILURE);
    } catch (...) {
        std::cerr << "Exception of unknown type!\n";
        killServer(EXIT_FAILURE);
    }
}

void seedDB() {
    int range = config["key-range"].as<int>();
    int size = config["size"].as<int>();
    while (size--) {
        int r = rand() % range;
        int v = rand();
        db[r] = v;
    }
    std::cout << "Database Seeded with " << db.size() << " Key-Value Pairs" << std::endl;
}

void *controller(void *_nsd) {
    int nsd = *((int *)_nsd);
    RequestType type;
    read(nsd, &type, sizeof(type));

    int responseDelay = config["response-delay"].as<int>();

    if (type == GET) {
        GetRequest request;
        read(nsd, &request, sizeof(request));
        GetResponse response;
        response.key = request.key;
        response.value = db.count(request.key) ? db[request.key] : -1;
        usleep(responseDelay * 1000);
        write(nsd, &response, sizeof(response));
    } else if (type == PUT) {
        PutRequest request;
        read(nsd, &request, sizeof(request));
        db[request.key] = request.value;
    } else if (type == ERASE) {
        EraseRequest request;
        read(nsd, &request, sizeof(request));
        db.erase(request.key);
    } else {
        perror("Unknown Request");
    }
    close(nsd);
    return NULL;
}

void startServer() {
    int nsd;
    signal(SIGINT, handle_my);
    socklen_t clientLen;
    pthread_t threads;
    struct sockaddr_in server, client;

    sd = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(5555);

    if (bind(sd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("bind failed");
        killServer(EXIT_FAILURE);
    }

    listen(sd, 5);

    printf("Waiting for the client...\n");

    clientLen = sizeof(client);

    while (1) {
        nsd = accept(sd, (struct sockaddr *)&client, &clientLen);

        if (pthread_create(&threads, NULL, controller, (void *)&nsd) < 0) {
            perror("pthread_create()");
            killServer(EXIT_FAILURE);
        }
    }

    pthread_exit(NULL);
}

int main(int ac, char *av[]) {
    initConfig(ac, av);
    seedDB();

    startServer();

    return 0;
}
