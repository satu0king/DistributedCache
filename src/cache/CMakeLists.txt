add_library(cache Cache.cpp LRU_cache_policy.cpp)

target_compile_options(cache PRIVATE -Wall)

target_include_directories(cache PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../headers ${CMAKE_CURRENT_SOURCE_DIR})
