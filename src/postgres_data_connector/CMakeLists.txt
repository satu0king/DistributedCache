add_library(postgres_data_connector postgres_data_connector.cpp)

target_include_directories(postgres_data_connector 
PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/../headers ${CMAKE_CURRENT_SOURCE_DIR}
PRIVATE  
    /usr/local/include
)

find_library(PQXX_LIB pqxx)
find_library(PQ_LIB pq)

target_link_libraries(postgres_data_connector ${PQXX_LIB} ${PQ_LIB})
