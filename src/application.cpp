
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
#include "postgres_data_connector.h"
#include "requests.h"

using namespace std;

unordered_map<int, int> db;
po::variables_map config;

void initConfig(int ac, char *av[]) {
    try {
        po::options_description desc("Allowed options");
        auto it = desc.add_options();
        it("help", "produce help message");
        it("db-port", po::value<int>()->default_value(5555), "Database Port");
        it("db-ip", po::value<string>()->default_value("127.0.0.1"),
           "Database IP");
        it("cache-port", po::value<int>()->default_value(6666), "Cache Port");
        it("cache-ip", po::value<string>()->default_value("127.0.0.1"),
           "Cache IP");

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

int main(int ac, char *av[]) {
    initConfig(ac, av);

    int db_port = config["db-port"].as<int>();
    std::string db_ip = config["db-ip"].as<std::string>();

    int cache_port = config["cache-port"].as<int>();
    std::string cache_ip = config["cache-ip"].as<std::string>();

    DataConnectorInterface *db = new BasicDataConnector(db_ip, db_port);
    // DataConnectorInterface *db = new PostgresDataConnector("mock_database");
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
