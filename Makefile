BUILD_DIR   = ./build
BIN_DIR     = ./bin
INCLUDE_DIR = ./include
SRC_DIR = ./src

CC          = g++ -std=c++17 -c -I $(INCLUDE_DIR)
LD          = g++ -std=c++17 -I $(INCLUDE_DIR)

SEED_SIZE = 100000

database: $(BIN_DIR)/database
	$(BIN_DIR)/database

application: $(BIN_DIR)/application 
	$(BIN_DIR)/application 

cache: $(BIN_DIR)/cache_server 
	$(BIN_DIR)/cache_server

$(BIN_DIR)/database: $(SRC_DIR)/mock_database.cpp $(INCLUDE_DIR)/requests.h
	$(LD) $(SRC_DIR)/mock_database.cpp -lboost_program_options -o $(BIN_DIR)/database

$(BIN_DIR)/application: $(SRC_DIR)/application.cpp $(INCLUDE_DIR)/requests.h $(BUILD_DIR)/basic_data_connector.o $(INCLUDE_DIR)/basic_data_connector.h
	$(LD) $(SRC_DIR)/application.cpp $(BUILD_DIR)/basic_data_connector.o -lboost_program_options -o $(BIN_DIR)/application

$(BIN_DIR)/cache_server: $(SRC_DIR)/cache/cache_server.cpp $(BUILD_DIR)/cache/cache.o $(BUILD_DIR)/basic_data_connector.o $(BUILD_DIR)/cache/LRU_cache_policy.o
	$(LD) $(SRC_DIR)/cache/cache_server.cpp $(BUILD_DIR)/basic_data_connector.o $(BUILD_DIR)/cache/LRU_cache_policy.o $(BUILD_DIR)/cache/cache.o -lboost_program_options -o $(BIN_DIR)/cache_server

$(BUILD_DIR)/basic_data_connector.o: $(SRC_DIR)/basic_data_connector.cpp $(INCLUDE_DIR)/requests.h $(INCLUDE_DIR)/basic_data_connector.h 
	$(CC) $(SRC_DIR)/basic_data_connector.cpp -o $(BUILD_DIR)/basic_data_connector.o

$(BUILD_DIR)/cache/cache.o: $(SRC_DIR)/cache/cache.cpp $(INCLUDE_DIR)/requests.h $(INCLUDE_DIR)/cache/cache.h 
	mkdir -p $(BUILD_DIR)/cache
	$(CC) $(SRC_DIR)/cache/cache.cpp -o $(BUILD_DIR)/cache/cache.o

$(BUILD_DIR)/cache/LRU_cache_policy.o: $(SRC_DIR)/cache/LRU_cache_policy.cpp $(INCLUDE_DIR)/requests.h $(INCLUDE_DIR)/cache/LRU_cache_policy.h 
	mkdir -p $(BUILD_DIR)/cache
	$(CC) $(SRC_DIR)/cache/LRU_cache_policy.cpp -o $(BUILD_DIR)/cache/LRU_cache_policy.o


clean:
	rm -rf bin/*
	rm -rf build/*