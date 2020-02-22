cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(RE VERSION 0.1)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Boost 1.72.0 COMPONENTS program_options REQUIRED)

add_subdirectory(src/basic_data_connector)
add_subdirectory(src/postgres_data_connector)
add_subdirectory(src/cache)

add_executable(app_server src/application.cpp)
target_link_libraries(app_server PRIVATE Boost::program_options basic_data_connector postgres_data_connector)

add_executable(cache_server src/cache/cache_server.cpp)
target_link_libraries(cache_server PRIVATE Boost::program_options cache basic_data_connector postgres_data_connector)

add_executable(mock_database src/mock_database.cpp)
target_link_libraries(mock_database PRIVATE Boost::program_options)