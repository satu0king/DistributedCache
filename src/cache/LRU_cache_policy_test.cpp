#include "LRU_cache_policy.h"

#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>

TEST_CASE("LRU Tests") {
    LRUCachePolicy policy(10);
    REQUIRE(policy.getSize() == 10);

    SECTION("LRU Insert Test") {
        int key = 10;
        int value = 125;
        std::string container = "container1";
        policy.insert(container, key, value);
        REQUIRE(policy.hasEntry(container, key));
        REQUIRE(policy.getEntry(container, key) == value);
    }

    SECTION("LRU Erase Test") {
        int key = 10;
        int value = 125;
        std::string container = "container1";
        policy.insert(container, key, value);
        policy.erase(container, key);
        REQUIRE(!policy.hasEntry(container, key));
    }

    SECTION("LRU Multiple Container Test") {
        std::string container1 = "container1";
        std::string container2 = "container2";
        int key = 10;
        int value1 = 125;
        int value2 = 126;
        policy.insert(container1, key, value1);
        policy.insert(container2, key, value2);

        REQUIRE(policy.hasEntry(container1, key));
        REQUIRE(policy.hasEntry(container2, key));

        REQUIRE(policy.getEntry(container1, key) == value1);
        REQUIRE(policy.getEntry(container2, key) == value2);
    }

    SECTION("LRU Reset Test") {
        int key = 10;
        int value = 125;
        std::string container = "container1";
        policy.insert(container, key, value);
        policy.reset();
        REQUIRE(!policy.hasEntry(container, key));
    }

    SECTION("LRU Policy Test") { 
        policy.allocateSize(5);
        std::vector<int> keys = {0, 1, 5, 4, 0, 3, 2};
        std::string container = "container";
        for(int i = 0; i < keys.size(); i++) {
            policy.insert(container, keys[i], i);
            REQUIRE(policy.hasEntry(container, keys[i]));
        }
        REQUIRE(!policy.hasEntry(container, 1));
        REQUIRE(policy.hasEntry(container, 0));
        REQUIRE(policy.hasEntry(container, 2));
        REQUIRE(policy.hasEntry(container, 3));
        REQUIRE(policy.hasEntry(container, 4));
        
        REQUIRE(policy.hasEntry(container, 5));
    }
}