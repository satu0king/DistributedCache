#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <boost/program_options.hpp>
#include <iostream>

#include "LRU_cache_policy.h"
#include "basic_data_connector.h"
#include "cache.h"
#include "postgres_data_connector.h"
#include "requests.h"
namespace po = boost::program_options;

po::variables_map config;
Cache *cache;

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

void initConfig(int ac, char *av[]) {
    try {
        po::options_description desc("Allowed options");
        auto init = desc.add_options();
        init("help", "produce help message");
        init("cache-size,s", po::value<int>()->default_value(100),
             "Cache Size");
        init("replacement-policy,p",
             po::value<std::string>()->default_value("LRU"),
             "Cache Replacement Policy");
        init("db-port", po::value<int>()->default_value(5555), "Database Port");
        init("db-ip", po::value<std::string>()->default_value("127.0.0.1"),
             "Database IP");
        init("cache-port", po::value<int>()->default_value(6666), "Cache Port");
        init("response-delay,d", po::value<int>()->default_value(10),
             "Reequest Delay in milliseconds");

        po::store(po::parse_command_line(ac, av, desc), config);
        po::notify(config);

        if (config.count("help")) {
            std::cout << desc << "\n";
            exit(0);
        }
    } catch (std::exception &e) {
        std::cerr << "error: " << e.what() << "\n";
        exit(EXIT_FAILURE);
    } catch (...) {
        std::cerr << "Exception of unknown type!\n";
        exit(EXIT_FAILURE);
    }
}

CachePolicyInterface *getCachePolicy() {
    std::string policy = config["replacement-policy"].as<std::string>();
    int size = config["cache-size"].as<int>();
    if (policy == "LRU") return new LRUCachePolicy(size);

    throw "Unknown Cache Policy: " + policy;
}

void initCache() {
    std::string databaseIP = config["db-ip"].as<std::string>();
    int databasePort = config["db-port"].as<int>();

    // DataConnectorInterface *database = new PostgresDataConnector("mock_database");

    DataConnectorInterface *database =
        new BasicDataConnector(databaseIP, databasePort);
        
    CachePolicyInterface *policy = getCachePolicy();
    cache = new Cache(policy, database);
}

void *controller(void *_nsd) {
    int nsd = *((int *)_nsd);
    delete (int *)_nsd;
    RequestType type;
    read(nsd, &type, sizeof(type));

    int responseDelay = config["response-delay"].as<int>();
    if (type == RequestType::GET) {
        GetRequest request;
        read(nsd, &request, sizeof(request));
        GetResponse response;
        response.value = cache->getEntry(request.container, request.key);
        usleep(responseDelay * 1000);
        write(nsd, &response, sizeof(response));
    } else if (type == RequestType::PUT) {
        PutRequest request;
        read(nsd, &request, sizeof(request));
        cache->insert(request.container, request.key, request.value);
    } else if (type == RequestType::RESET) {
        cache->reset();
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

    int port = config["cache-port"].as<int>();

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);

    if (bind(sd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("bind failed");
        killServer(EXIT_FAILURE);
    }

    listen(sd, 5);

    printf("Waiting for the client...\n");

    clientLen = sizeof(client);

    while (1) {
        nsd = accept(sd, (struct sockaddr *)&client, &clientLen);
        int *socketDescriptor = new int(nsd);
        if (pthread_create(&threads, NULL, controller, socketDescriptor) < 0) {
            perror("pthread_create()");
            killServer(EXIT_FAILURE);
        }
    }

    pthread_exit(NULL);
}

int main(int argc, char **argv) {
    initConfig(argc, argv);
    initCache();
    startServer();
}
