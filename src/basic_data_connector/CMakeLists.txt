add_library(basic_data_connector basic_data_connector.cpp)

target_include_directories(basic_data_connector PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(basic_data_connector PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
