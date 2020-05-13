
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
#include <queue>
#include <thread>
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

template <typename T>
class WorkerPool {
    std::function<void(T)> executor;
    std::queue<T> queue;
    std::mutex queueLock;
    std::vector<std::thread> threads;
    int threadCount;

   public:
    WorkerPool(std::function<void(T)> executor, int threadCount = 0,
               bool exitWhenDone = true)
        : executor(executor), threadCount(threadCount) {}

    void insert(T obj) {
        std::lock_guard<std::mutex> guard(queueLock);
        queue.push(obj);
    }

    void insert(std::vector<T> &objects) {
        std::lock_guard<std::mutex> guard(queueLock);
        for (auto &obj : objects) queue.push(obj);
    }

    void start(int c = 0) {
        threadCount += c;

        while (threadCount--)
            threads.push_back(std::thread(&WorkerPool::threadRunner, this));
    }

    void threadRunner() {
        while (1) {
            T obj;
            {
                std::lock_guard<std::mutex> guard(queueLock);
                // queueLock.lock();
                if (queue.empty()) {
                    return;
                }
                if (queue.size() % 100 == 0)
                    ;
                std::cout << queue.size() << endl;
                obj = queue.front();
                queue.pop();
                // queueLock.unlock();
            }

            executor(obj);
        }
    }

    void join() {
        for (auto &thread : threads) thread.join();
    }
};

int main(int ac, char *av[]) {
    initConfig(ac, av);
    srand(time(NULL));

    std::cout << "Starting Tests" << std::endl;

    int db_port = config["db-port"].as<int>();
    std::string db_ip = config["db-ip"].as<std::string>();

    // int cache_port = config["cache-port"].as<int>();
    // std::string cache_ip = config["cache-ip"].as<std::string>();
    std::string cache_ip = "127.0.0.1";

    // DataConnectorInterface *db = new BasicDataConnector(db_ip, db_port);
    DataConnectorInterface *db = getDataConnector();
    // DataConnectorInterface *cache =
    //     new BasicDataConnector(cache_ip, cache_port);

    std::vector<DataConnectorInterface *> caches;
    caches.push_back(new BasicDataConnector(cache_ip, 6666));
    for (int port = 7001; port <= 7019; port++) {
        caches.push_back(new BasicDataConnector(cache_ip, port));
    }

    db->reset();
    for (auto cache : caches) cache->reset();

    std::string containerName = "CONTAINER1";

    int n = 1000;
    std::cout << "Seeding Database" << std::endl;
    vector<int> values(n);

    using milli = std::chrono::milliseconds;

    for (int i = 0; i < n; i++) {
        values[i] = rand() % 1000000;
        if (i % 100 == 99) std::cout << i + 1 << std::endl;
        db->put(containerName, i, values[i]);
    }

    std::cout << "Seeding Database Done" << std::endl;

    float probability = 0.5;

    int q = 5000;

    vector<int> random(100);

    double totalTime = 0;

    std::deque<int> queue;

    std::cout << "Running Queries" << std::endl;

    for (int i = 0; i < 100; i++) {
        random[i] = rand() % n;
    }

    for (int i = 0; i < q - 100; i++) {
        int j = rand() % 100;
        int p = rand() % 10000;
        if (p > probability * 10000) {
            random.push_back(rand() % n);
        } else {
            random.push_back(random[random.size() - j - 1]);
        }
    }

    auto CacheTester = [&](int key) {
        // std::cout << key << std::endl;
        int v = caches[rand() % caches.size()]->get(containerName, key);
        // if(v != values[key]) {
        //     std::cout << v << std::endl;
        //     std::cout << values[key] << std::endl;
        // }
        assert(v == values[key]);
    };
    WorkerPool<int> workerPool(CacheTester);
    std::cout << random.size() << std::endl;
    workerPool.insert(random);

    auto start = std::chrono::high_resolution_clock::now();
    workerPool.start(10);
    workerPool.join();
    auto finish = std::chrono::high_resolution_clock::now();

    auto time =
        std::chrono::duration_cast<std::chrono::milliseconds>(finish - start)
            .count();
    std::cout << "Total Time : " << time << std::endl;

    for (int i = 0; i < q; i++) {
        int j = rand() % 100;

        int p = rand() % 10000;
        if (p > probability * 10000) {
            random[j] = rand() % n;
        }

        auto start = std::chrono::high_resolution_clock::now();
        int key = random[j];
        int v = caches[rand() % caches.size()]->get(containerName, key);
        assert(v == values[key]);
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
