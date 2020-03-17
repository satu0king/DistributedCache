add_library(membership_list membership_list.cpp)

target_compile_options(membership_list PRIVATE -Wall)

target_include_directories(membership_list PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../headers ${CMAKE_CURRENT_SOURCE_DIR})

