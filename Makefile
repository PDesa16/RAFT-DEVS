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
    models/test/raft_test.cpp \
    vendor/googletest/lib/libgtest.a \
    vendor/googletest/lib/libgtest_main.a \
    -o bin/test_raft \
    -pthread


