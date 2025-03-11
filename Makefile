# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -g --std=c++17 -pthread

# Directories
VENDOR_DIR = vendor
INCLUDE_DIRS = -I$(VENDOR_DIR)/cadmium_v2/include \
               -I$(VENDOR_DIR)/googletest/googletest/include \
               -I$(VENDOR_DIR)/googletest/googlemock/include \
               -I$(VENDOR_DIR)/cryptopp

LIB_DIRS = -L$(VENDOR_DIR)/googletest/lib -L$(VENDOR_DIR)/cryptopp

GTEST_LIBS = $(VENDOR_DIR)/googletest/lib/libgtest.a \
             $(VENDOR_DIR)/googletest/lib/libgtest_main.a

CRYPTOPP_LIBS = $(VENDOR_DIR)/cryptopp/libcryptopp.a

BIN_DIR = bin
SRC_DIR = models
UTILS_DIR = utils

# Test targets
TESTS = test_buffer test_network \
        test_raft test_packet_processor test_message_processor \
        test_node test_heartbeat_controller test_simulation test_raft_controller

# Build and run all tests
all: $(TESTS) run_tests

build_test_raft_controller:
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) $(SRC_DIR)/atomic/test/raft_test.cpp \
		$(UTILS_DIR)/cryptography/crypto.cpp $(UTILS_DIR)/stochastic/random.cpp \
		$(GTEST_LIBS) $(CRYPTOPP_LIBS) -o $(BIN_DIR)/test_raft_controller $(LIB_DIRS)

build_test_network:
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) $(SRC_DIR)/atomic/test/network_test.cpp \
		$(UTILS_DIR)/stochastic/random.cpp $(GTEST_LIBS) $(CRYPTOPP_LIBS) -o $(BIN_DIR)/test_network $(LIB_DIRS)

build_packet_processor_raft:
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) $(SRC_DIR)/atomic/test/packet_processor_test.cpp \
		$(UTILS_DIR)/cryptography/crypto.cpp $(UTILS_DIR)/stochastic/random.cpp \
		$(GTEST_LIBS) $(CRYPTOPP_LIBS) -o $(BIN_DIR)/test_packet_processor $(LIB_DIRS)

build_message_processor_raft:
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) $(SRC_DIR)/atomic/test/message_processor_test.cpp \
		$(UTILS_DIR)/cryptography/crypto.cpp $(UTILS_DIR)/stochastic/random.cpp \
		$(GTEST_LIBS) $(CRYPTOPP_LIBS) -o $(BIN_DIR)/test_message_processor $(LIB_DIRS)

build_node:
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) $(SRC_DIR)/coupled/test/node_test.cpp \
		$(UTILS_DIR)/cryptography/crypto.cpp $(UTILS_DIR)/stochastic/random.cpp \
		$(GTEST_LIBS) $(CRYPTOPP_LIBS) -o $(BIN_DIR)/test_node $(LIB_DIRS)

build_simulation:
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) $(SRC_DIR)/coupled/test/simulation_test.cpp \
		$(UTILS_DIR)/cryptography/crypto.cpp $(UTILS_DIR)/stochastic/random.cpp \
		$(GTEST_LIBS) $(CRYPTOPP_LIBS) -o $(BIN_DIR)/test_simulation $(LIB_DIRS)

build_heartbeat_controller:
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) $(SRC_DIR)/atomic/test/heartbeat_controller_test.cpp \
		$(UTILS_DIR)/cryptography/crypto.cpp $(UTILS_DIR)/stochastic/random.cpp \
		$(GTEST_LIBS) $(CRYPTOPP_LIBS) -o $(BIN_DIR)/test_heartbeat_controller $(LIB_DIRS)

build_raft:
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) $(SRC_DIR)/coupled/test/raft_test.cpp \
		$(UTILS_DIR)/cryptography/crypto.cpp $(UTILS_DIR)/stochastic/random.cpp \
		$(GTEST_LIBS) $(CRYPTOPP_LIBS) -o $(BIN_DIR)/test_raft $(LIB_DIRS)

build_buffer:
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) $(SRC_DIR)/atomic/test/buffer_test.cpp \
		$(UTILS_DIR)/cryptography/crypto.cpp $(UTILS_DIR)/stochastic/random.cpp \
		$(GTEST_LIBS) $(CRYPTOPP_LIBS) -o $(BIN_DIR)/test_buffer $(LIB_DIRS)


# Run all tests
run_tests: $(addprefix run_, $(TESTS))

run_test_buffer:
	$(BIN_DIR)/test_buffer

run_test_raft_controller:
	$(BIN_DIR)/test_raft_controller

run_test_network:
	$(BIN_DIR)/test_network

run_test_raft:
	$(BIN_DIR)/test_raft

run_test_packet_processor:
	$(BIN_DIR)/test_packet_processor

run_test_message_processor:
	$(BIN_DIR)/test_message_processor

run_test_node:
	$(BIN_DIR)/test_node

run_test_simulation:
	$(BIN_DIR)/test_simulation

run_test_heartbeat_controller:
	$(BIN_DIR)/test_heartbeat_controller

run_test_raft:
	$(BIN_DIR)/test_raft

.PHONY: all $(TESTS) run_tests

clean:
	rm -rf $(BIN_DIR)/*

build_all: build_test_raft_controller build_test_network build_packet_processor_raft \
           build_message_processor_raft build_node build_simulation build_heartbeat_controller \
           build_raft build_buffer
