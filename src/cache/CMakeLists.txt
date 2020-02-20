add_library(cache Cache.cpp LRU_cache_policy.cpp)

target_include_directories(cache PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
