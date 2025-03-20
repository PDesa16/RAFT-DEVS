# Project Setup and Usage

## Cloning the Repository
To clone this repository along with its submodules, run:
```sh
git clone --recurse-submodules https://github.com/PDesa16/RAFT-DEVS
```

## Traverse to Project Directory
```sh
cd RAFT-DEVS
```
## Give Permissions to the Build Script
```sh
chmod 755 build_script.sh
```


## Building Dependencies
Before building the project, install the necessary dependencies by running:
```sh
./build_dependencies.sh
```

## Building the Project
To build all tests, use:
```sh
make build_all
```

You can also build individual components using:
```sh
make build_test_raft_controller
make build_test_network
make build_packet_processor_raft
make build_message_processor_raft
make build_node
make build_simulation
make build_heartbeat_controller
make build_raft
make build_buffer
```

## Running Tests
To run all tests:
```sh
make run_tests
```

To run specific tests:
```sh
make run_test_buffer
make run_test_raft_controller
make run_test_network
make run_test_raft
make run_test_packet_processor_raft
make run_test_message_processor_raft
make run_test_node
make run_test_simulation
make run_test_heartbeat_controller
make run_test_raft
```

## Running the Simulation
To run the simulation test, execute:
```sh
make run_test_simulation
```

## Cleaning Up
To clean up compiled binaries, run:
```sh
make clean
```

## Modifying and Rebuilding
Modify the source files as needed and then rebuild using:
```sh
make <target>
```
For example, if you modified the `simulation_test.cpp` file, rebuild it with:
```sh
make build_simulation
```

