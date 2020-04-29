cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(DistributedCache VERSION 0.1)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Boost 1.65.1 COMPONENTS program_options REQUIRED)
find_package(Catch2 REQUIRED)

add_subdirectory(src/basic_data_connector)
add_subdirectory(src/postgres_data_connector)
add_subdirectory(src/cache)
add_subdirectory(src/gossip)
add_subdirectory(src/membership_list)

add_executable(app_server src/application.cpp)
target_link_libraries(app_server PRIVATE Boost::program_options basic_data_connector postgres_data_connector)

add_executable(cache_server src/cache/cache_server.cpp)
target_link_libraries(cache_server PRIVATE Boost::program_options cache basic_data_connector postgres_data_connector gossip membership_list)

add_executable(cache_policy_lru_test src/cache/LRU_cache_policy_test.cpp)
target_link_libraries(cache_policy_lru_test PRIVATE cache Catch2::Catch2)

add_executable(mock_database src/mock_database.cpp)
target_link_libraries(mock_database PRIVATE Boost::program_options)