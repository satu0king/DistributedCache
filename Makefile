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

$(BIN_DIR)/database: $(SRC_DIR)/mock_database.cpp $(INCLUDE_DIR)/requests.h
	$(LD) $(SRC_DIR)/mock_database.cpp -lboost_program_options -o $(BIN_DIR)/database

$(BIN_DIR)/application: $(SRC_DIR)/application.cpp $(INCLUDE_DIR)/requests.h
	$(LD) $(SRC_DIR)/application.cpp -lboost_program_options -o $(BIN_DIR)/application



clean:
	rm bin/*
	rm build/*