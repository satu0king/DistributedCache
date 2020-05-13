#include <fcntl.h>
#include <netinet/in.h>
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
#include "cache_policies.h"
#include "data_connectors.h"
#include "member_node.h"
#include "membership.h"
#include "postgres_data_connector.h"
#include "requests.h"
#include "server_interface.h"
#include "server_thread_pool.h"


namespace po = boost::program_options;

po::variables_map config;

void initConfig(int ac, char *av[]) {
    try {
        po::options_description desc("Allowed options");
        auto init = desc.add_options();

        init("help", "produce help message");
        init("cache-size,s", po::value<int>()->default_value(100),
             "Cache Size");
        init("replacement-policy,p",
             po::value<CachePolicy>()->default_value(CachePolicy::LRU, "LRU"),
             "Cache Replacement Policy");
        init("data-source",
             po::value<DataConnector>()->default_value(DataConnector::DEFAULT,
                                                       "DEFAULT"),
             "Database connector [DEFAULT | POSTGRES]");
        init("db-name",
             po::value<std::string>()->default_value("mock_database"),
             "Database Name");
        init("db-port", po::value<int>()->default_value(5555), "Database Port");
        init("db-ip", po::value<std::string>()->default_value("127.0.0.1"),
             "Database IP");
        init("cache-port", po::value<int>()->default_value(6666), "Cache Port");
        init("ring-start-range", po::value<int>()->default_value(0),
             "Consistent Hashing ring Range");
        init("cache-ip", po::value<std::string>()->default_value("127.0.0.1"),
             "Cache ip");
        init("introducer-port", po::value<int>(), "introducer Port");
        init("introducer-ip", po::value<std::string>(), "introducer ip");
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
    CachePolicy policy = config["replacement-policy"].as<CachePolicy>();
    int size = config["cache-size"].as<int>();

    switch (policy) {
        case CachePolicy::LRU:
            return new LRUCachePolicy(size);
    }

    throw std::runtime_error("Unknown CachePolicy");
}

DataConnectorInterface *getDataConnector() {
    DataConnector dataConnector = config["data-source"].as<DataConnector>();

    if (dataConnector == DataConnector::DEFAULT) {
        std::string databaseIP = config["db-ip"].as<std::string>();
        int databasePort = config["db-port"].as<int>();
        return new BasicDataConnector(databaseIP, databasePort);
    } else if (dataConnector == DataConnector::POSTGRES) {
        std::string databaseName = config["db-name"].as<std::string>();
        return new PostgresDataConnector(databaseName);
    }

    throw std::runtime_error("Unknown CachePolicy");
}

void killServer(int ret);

class CacheServer : public MultiThreadedServerInterface {
    DataConnectorInterface *database;
    CachePolicyInterface *policy;
    Cache *cache;

    MemberNode *member;

    ServerThreadPool *pool;

   public:
    CacheServer() : pool(NULL), member(NULL), cache(NULL) {}

    void controller(int nsd) {
        RequestType type;

        try {
            loop_read(nsd, &type, sizeof(type));

            int responseDelay = config["response-delay"].as<int>();

            auto address = member->getAddress();

            if (type == RequestType::GET) {
                GetRequest request;
                read(nsd, &request, sizeof(request));
                // std::cout << address.toString() << " GET " << request.key <<
                // std::endl;
                Address targetNode = member->getNearestNode(
                    request.key);  // Get Target node from consistent ring

                if (targetNode == address) {
                    GetResponse response;
                    response.value =
                        getCache()->getEntry(request.container, request.key);
                    usleep(responseDelay * 1000);
                    write(nsd, &response, sizeof(response));
                    return;
                }

                int connection = Address::getConnection(targetNode);
                if (connection == -1) {
                    perror("Connection to Target Node Failed");
                    return;
                }
                RequestType type = RequestType::GET_TARGET;  // Forward request
                loop_write(connection, &type, sizeof(type));
                loop_write(connection, &request, sizeof(request));

                GetResponse response;  // Get response
                read(connection, &response, sizeof(response));
                close(connection);

                write(nsd, &response, sizeof(response));  // Return Response
            } else if (type == RequestType::PUT) {
                PutRequest request;
                read(nsd, &request, sizeof(request));
                Address targetNode = member->getNearestNode(request.key);
                std::cout << address.toString() << " PUT " << request.key << " "
                          << request.value << std::endl;
                int connection = Address::getConnection(targetNode);
                if (connection == -1) {
                    perror("Connection to Target Node Failed");
                    return;
                }
                RequestType type = RequestType::PUT_TARGET;
                loop_write(connection, &type, sizeof(type));
                loop_write(connection, &request, sizeof(request));
                close(connection);
            } else if (type == RequestType::GET_TARGET) {
                GetRequest request;
                read(nsd, &request, sizeof(request));
                // std::cout << address.toString() << " GET_TARGET " <<
                // request.key
                // << std::endl;
                GetResponse response;
                response.value =
                    getCache()->getEntry(request.container, request.key);
                usleep(responseDelay * 1000);
                write(nsd, &response, sizeof(response));
            } else if (type == RequestType::PUT_TARGET) {
                PutRequest request;
                read(nsd, &request, sizeof(request));
                std::cout << address.toString() << " PUT_TARGET " << request.key
                          << " " << request.value << std::endl;
                getCache()->insert(request.container, request.key,
                                   request.value);
            } else if (type == RequestType::RESET) {
                getCache()->reset();
            } else if (type == RequestType::GOSSIP) {
                member->receiveGossip(nsd);
            } else {
                throw std::runtime_error("Unknown request type");
            }
        }
        catch (const std::exception &e) {
            std::cerr << "error: " << e.what() << "\n";
        }
    }

    Cache *getCache() { return cache; }
    void initCache() {
        DataConnectorInterface *database = getDataConnector();
        CachePolicyInterface *policy = getCachePolicy();
        cache = new Cache(policy, database);
    }

    void start() {
        int port = config["cache-port"].as<int>();
        std::string ip = config["cache-ip"].as<std::string>();
        int startRange = config["ring-start-range"].as<int>();

        pool = new ServerThreadPool(this, ip, port);

        member = new MemberNode(Address(ip, port), startRange);

        if (config.count("introducer-ip") && config.count("introducer-port")) {
            auto introducer_ip = config["introducer-ip"].as<std::string>();
            auto introducer_port = config["introducer-port"].as<int>();
            member->addIntroducer(introducer_ip, introducer_port);
        }

        printf("Waiting for the client...\n");
        pool->start();
    }

    void kill() {
        if (pool) pool->stop();
    }
};

CacheServer server;

void killServer(int ret = EXIT_SUCCESS) {
    std::cout << "Shutting Down Server ...\n";
    server.kill();
    exit(ret);
}

void handle_my(int sig) {
    switch (sig) {
        case SIGINT:
            killServer();
            break;
    }
}

int main(int argc, char **argv) {
    initConfig(argc, argv);
    signal(SIGINT, handle_my);

    server.initCache();
    server.start();
}
