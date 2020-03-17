add_library(gossip member_node.cpp)

target_compile_options(gossip PRIVATE -Wall)

target_include_directories(gossip 

PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../headers ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../membership_list)

