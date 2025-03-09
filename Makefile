build_debug:
	# g++ -Wall -g --std=c++17 -c main.cpp -o bin/main.o
	# g++ -Wall -g --std=c++17 -Ivendor/yaml-cpp/include -c config/config.cpp -o bin/config.o
	g++ -Wall -g --std=c++17 -Wno-c++20-compat -Ivendor/cadmium/include -Ivendor/json/include -c models/ntp_model.cpp -o bin/ntp_model.o
	# # g++ -Wall -g --std=c++17 -c ntp/ntp.cpp -o bin/ntp.o
	# # g++ -Wall -g --std=c++17 -c ntp/messages.cpp -o bin/messages.o


build_test_buffer:
	g++ -Wall -g --std=c++17 \
    -Ivendor/cadmium_v2/include \
    -Ivendor/googletest/googletest/include \
    -Ivendor/googletest/googlemock/include \
    models/test/buffer_test.cpp \
    vendor/googletest/lib/libgtest.a \
    vendor/googletest/lib/libgtest_main.a \
    -o bin/test_buffer \
    -pthread

build_test_database:
	g++ -Wall -g --std=c++17 \
    -Ivendor/cadmium_v2/include \
    -Ivendor/googletest/googletest/include \
    -Ivendor/googletest/googlemock/include \
    models/test/database_test.cpp \
    vendor/googletest/lib/libgtest.a \
    vendor/googletest/lib/libgtest_main.a \
    -o bin/test_database \
    -pthread

build_test_raft:
	g++ -Wall -g --std=c++17 \
    -Ivendor/cadmium_v2/include \
    -Ivendor/googletest/googletest/include \
    -Ivendor/googletest/googlemock/include \
    -Ivendor/cryptopp \
    models/test/raft_test.cpp \
    utils/cryptography/crypto.cpp \
    utils/stochastic/random.cpp \
    vendor/googletest/lib/libgtest.a \
    vendor/googletest/lib/libgtest_main.a \
    vendor/cryptopp/libcryptopp.a \
    -o bin/test_raft \
    -pthread

build_test_network:
	g++ -Wall -g --std=c++17 \
    -Ivendor/cadmium_v2/include \
    -Ivendor/googletest/googletest/include \
    -Ivendor/googletest/googlemock/include \
    -Ivendor/cryptopp \
    models/test/network_test.cpp \
    utils/stochastic/random.cpp \
    vendor/googletest/lib/libgtest.a \
    vendor/googletest/lib/libgtest_main.a \
    vendor/cryptopp/libcryptopp.a \
    -o bin/test_network \
    -pthread

build_test_buffer_raft:
	g++ -Wall -g --std=c++17 \
    -Ivendor/cadmium_v2/include \
    -Ivendor/googletest/googletest/include \
    -Ivendor/googletest/googlemock/include \
    -Ivendor/cryptopp \
    models/coupled/test/buffer_raft_test.cpp \
    utils/cryptography/crypto.cpp \
    utils/stochastic/random.cpp \
    vendor/googletest/lib/libgtest.a \
    vendor/googletest/lib/libgtest_main.a \
    vendor/cryptopp/libcryptopp.a \
    -o bin/test_buffer_raft \
    -pthread

build_packet_processor_raft:
	g++ -Wall -g --std=c++17 \
    -Ivendor/cadmium_v2/include \
    -Ivendor/googletest/googletest/include \
    -Ivendor/googletest/googlemock/include \
    -Ivendor/cryptopp \
    models/test/packet_processor_test.cpp \
    utils/cryptography/crypto.cpp \
    utils/stochastic/random.cpp \
    vendor/googletest/lib/libgtest.a \
    vendor/googletest/lib/libgtest_main.a \
    vendor/cryptopp/libcryptopp.a \
    -o bin/test_packet_processor \
    -pthread


build_message_processor_raft:
	g++ -Wall -g --std=c++17 \
    -Ivendor/cadmium_v2/include \
    -Ivendor/googletest/googletest/include \
    -Ivendor/googletest/googlemock/include \
    -Ivendor/cryptopp \
    models/test/message_processor_test.cpp \
    utils/cryptography/crypto.cpp \
    utils/stochastic/random.cpp \
    vendor/googletest/lib/libgtest.a \
    vendor/googletest/lib/libgtest_main.a \
    vendor/cryptopp/libcryptopp.a \
    -o bin/test_message_processor \
    -pthread



build_node:
	g++ -Wall -g --std=c++17 \
    -Ivendor/cadmium_v2/include \
    -Ivendor/googletest/googletest/include \
    -Ivendor/googletest/googlemock/include \
    -Ivendor/cryptopp \
    models/coupled/test/node_test.cpp \
    utils/cryptography/crypto.cpp \
    utils/stochastic/random.cpp \
    vendor/googletest/lib/libgtest.a \
    vendor/googletest/lib/libgtest_main.a \
    vendor/cryptopp/libcryptopp.a \
    -o bin/test_node \
    -pthread


build_simulation:
	g++ -Wall -g --std=c++17 \
    -Ivendor/cadmium_v2/include \
    -Ivendor/googletest/googletest/include \
    -Ivendor/googletest/googlemock/include \
    -Ivendor/cryptopp \
    models/coupled/test/simulation_test.cpp \
    utils/cryptography/crypto.cpp \
    utils/stochastic/random.cpp \
    vendor/googletest/lib/libgtest.a \
    vendor/googletest/lib/libgtest_main.a \
    vendor/cryptopp/libcryptopp.a \
    -o bin/test_simulation \
    -pthread