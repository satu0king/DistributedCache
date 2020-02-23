
#include <boost/program_options.hpp>
namespace po = boost::program_options;
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <chrono>
#include <iostream>
#include <iterator>
#include <unordered_map>

#include "basic_data_connector.h"
#include "data_connectors.h"
#include "postgres_data_connector.h"
#include "requests.h"

using namespace std;

unordered_map<int, int> db;
po::variables_map config;

void initConfig(int ac, char *av[]) {
    try {
        po::options_description desc("Allowed options");
        auto init = desc.add_options();
        init("help", "produce help message");
        init("db-port", po::value<int>()->default_value(5555), "Database Port");
        init("db-ip", po::value<string>()->default_value("127.0.0.1"),
             "Database IP");
        init("cache-port", po::value<int>()->default_value(6666), "Cache Port");
        init("cache-ip", po::value<string>()->default_value("127.0.0.1"),
             "Cache IP");
        init("db-name",
             po::value<std::string>()->default_value("mock_database"),
             "Database Name");
        init("data-source",
             po::value<DataConnector>()->default_value(DataConnector::DEFAULT,
                                                       "DEFAULT"),
             "Database connector [DEFAULT | POSTGRES]");

        po::store(po::parse_command_line(ac, av, desc), config);
        po::notify(config);

        if (config.count("help")) {
            cout << desc << "\n";
            exit(0);
        }
    } catch (exception &e) {
        cerr << "error: " << e.what() << "\n";
        exit(EXIT_FAILURE);
    } catch (...) {
        cerr << "Exception of unknown type!\n";
        exit(EXIT_FAILURE);
    }
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

int main(int ac, char *av[]) {
    initConfig(ac, av);

    int db_port = config["db-port"].as<int>();
    std::string db_ip = config["db-ip"].as<std::string>();

    int cache_port = config["cache-port"].as<int>();
    std::string cache_ip = config["cache-ip"].as<std::string>();

    // DataConnectorInterface *db = new BasicDataConnector(db_ip, db_port);
    DataConnectorInterface *db = getDataConnector();
    DataConnectorInterface *cache =
        new BasicDataConnector(cache_ip, cache_port);

    // db->reset();
    cache->reset();

    std::string containerName = "CONTAINER1";

    int n = 1000;
    std::cout << "Seeding Database" << std::endl;

    for (int i = 0; i < n; i++) {
        db->put(containerName, i, i);
        if (i % 100 == 99) std::cout << i + 1 << std::endl;
    }

    vector<int> random(n);

    for (int i = 0; i < 100; i++) {
        random[i] = i;
    }

    float probability = 0.8;

    int q = 1000;

    using milli = std::chrono::milliseconds;

    double totalTime = 0;

    std::deque<int> queue;

    std::cout << "Running Queries" << std::endl;

    for (int i = 0; i < q; i++) {
        int j = rand() % 100;

        int p = rand() % 10000;
        if (p > probability * 10000) {
            random[j] = rand() % n;
        }

        auto start = std::chrono::high_resolution_clock::now();
        int v = cache->get(containerName, random[j]);
        assert(v == random[j]);
        auto finish = std::chrono::high_resolution_clock::now();

        auto time = std::chrono::duration_cast<milli>(finish - start).count();

        queue.push_back(time);
        totalTime += time;
        if (queue.size() > 100) {
            totalTime -= queue.front();
            queue.pop_front();
        }

        if (i % 50 == 49)
            std::cout << i + 1 << " - Average Query Time is "
                      << totalTime / queue.size() << " milliseconds\n";
    }

    return 0;
}
