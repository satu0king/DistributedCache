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
#include "requests.h"

using namespace std;

unordered_map<int, int> db;
po::variables_map config;

void initConfig(int ac, char *av[]) {
    try {
        po::options_description desc("Allowed options");
        desc.add_options()("help", "produce help message")
        ("DB-Port", po::value<int>()->default_value(5555), "Database Port")
        ("DB-IP", po::value<string>()->default_value("127.0.0.1"), "Database IP");

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

int connectToDB() {
    int IP = inet_addr(config["DB-IP"].as<string>().c_str());  // INADDR_ANY;
    int port = config["DB-Port"].as<int>();
    struct sockaddr_in server, client;
    int sd = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = IP;
    server.sin_port = htons(5555);

    if (connect(sd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect()");
        exit(0);
    }

    return sd;
}

void insertToDB(int key, int value) {
    int sd = connectToDB();
    RequestType type = PUT;
    write(sd, &type, sizeof(type));
    PutRequest request = {.key = key, .value = value};
    write(sd, &request, sizeof(request));
    close(sd);
}

int getFromDB(int key) {
    int sd = connectToDB();
    RequestType type = GET;
    write(sd, &type, sizeof(type));
    GetRequest request = {.key = key};
    write(sd, &request, sizeof(request));

    GetResponse response;
    read(sd, &response, sizeof(response));
    close(sd);
    return response.value;
}

int main(int ac, char *av[]) {
    initConfig(ac, av);

    insertToDB(100, 123);
    cout << getFromDB(100) << endl;
    cout << getFromDB(101) << endl;

    return 0;
}
