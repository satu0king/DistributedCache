

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

#include "headers/requests.h"
#include "headers/types.h"
#include "multithreaded_server/server_interface.h"
#include "multithreaded_server/server_thread_pool.h"


po::variables_map config;

class MockDatabase : public MultiThreadedServerInterface {
    ServerThreadPool *pool;

    std::map<key_pair_t, int> db;
    std::mutex dbLock;

   public:
    void seedDB() {
        int range = config["key-range"].as<int>();
        int size = config["size"].as<int>();
        while (size--) {
            int r = rand() % range;
            int v = rand();
            db[{"CONTAINER1", r}] = v;
        }
        std::cout << "Database Seeded with " << db.size() << " Key-Value Pairs"
                  << std::endl;
    }
    void controller(int nsd) {
        RequestType type;
        loop_read(nsd, &type, sizeof(type));

        int responseDelay = config["response-delay"].as<int>();

        if (type == RequestType::GET) {
            usleep(responseDelay * 1000);
            std::lock_guard<std::mutex> guard(dbLock);
            GetRequest request;
            loop_read(nsd, &request, sizeof(request));
            auto key_pair =
                make_pair(std::string(request.container), request.key);
            GetResponse response;
            response.value = db.count(key_pair) ? db[key_pair] : -1;
            loop_write(nsd, &response, sizeof(response));
        } else if (type == RequestType::PUT) {
            std::lock_guard<std::mutex> guard(dbLock);
            PutRequest request;
            loop_read(nsd, &request, sizeof(request));
            auto key_pair =
                make_pair(std::string(request.container), request.key);
            db[key_pair] = request.value;
        } else if (type == RequestType::ERASE) {
            std::lock_guard<std::mutex> guard(dbLock);
            EraseRequest request;
            loop_read(nsd, &request, sizeof(request));
            auto key_pair =
                make_pair(std::string(request.container), request.key);
            db.erase(key_pair);
        } else if (type == RequestType::RESET) {
            std::lock_guard<std::mutex> guard(dbLock);
            db.clear();
        } else {
            std::cout << "Unknown Request: " << static_cast<int>(type)
                      << std::endl;
        }
    }

    void start() {
        int port = config["db-port"].as<int>();
        std::string ip = config["db-ip"].as<std::string>();
        pool = new ServerThreadPool(this, ip, port, 20, 40);
        pool->start();
    }

    void kill() {
        if (pool) pool->stop();
    }
};

MockDatabase db;
void killServer(int ret = EXIT_SUCCESS) {
    std::cout << "Shutting Down Server ...\n";
    db.kill();
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
        init("size,s", po::value<int>()->default_value(0),
             "Initial seed size of the database");
        init("key-range,r", po::value<int>()->default_value(1 << 20),
             "Maximum Key Value");
        init("response-delay,d", po::value<int>()->default_value(50),
             "Reequest Delay in milliseconds");
        init("db-ip", po::value<std::string>()->default_value("127.0.0.1"),
             "Database ip");
        init("db-port", po::value<int>()->default_value(5555), "Database Port");

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


int main(int ac, char *av[]) {
    initConfig(ac, av);
    signal(SIGINT, handle_my);

    MockDatabase db;
    db.seedDB();

    db.start();

    return 0;
}
