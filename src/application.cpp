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
#include <sys/socket.h>
#include <sys/types.h>
#include <iostream>
#include <iterator>
#include <unordered_map>
#include "mock_database_connector.h"
#include "requests.h"

using namespace std;

unordered_map<int, int> db;
po::variables_map config;

void initConfig(int ac, char *av[]) {
    try {
        po::options_description desc("Allowed options");
        desc.add_options()("help", "produce help message")("DB-Port", po::value<int>()->default_value(5555),
                                                           "Database Port")(
            "DB-IP", po::value<string>()->default_value("127.0.0.1"), "Database IP");

        po::store(po::parse_command_line(ac, av, desc), config);
        po::notify(config);

        if (config.count("help")) {
            cout << desc << "\n";
            exit(0);
        }
    } catch (exception &e) {
        cerr << "error: " << e.what() << "\n";
        exit(0);
    } catch (...) {
        cerr << "Exception of unknown type!\n";
        exit(0);
    }
}

int main(int ac, char *av[]) {
    initConfig(ac, av);

    DatabaseConnectorInterface *DB =
        new MockDatabaseConnector(config["DB-IP"].as<std::string>(), config["DB-Port"].as<int>());
    
    DB->put(3, 4);
    std::cout << DB->get(3) << std::endl;
    std::cout << DB->get(5) << std::endl;

    return 0;
}
