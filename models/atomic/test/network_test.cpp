#include <gtest/gtest.h>
#include <cadmium/core/modeling/atomic.hpp>
#include "../../models/atomic/network.hpp"
#include "../../messages/network/network_message.hpp"
#include "../../messages/raft/raft_messages.hpp"

using namespace cadmium;

class NetworkAtomicFixture : public ::testing::Test {
protected:
    NetworkModel* model;
    NetworkState state;

    void SetUp() override {
        // Initialize the model with some nodes
        std::vector<std::string> nodes = {"node0", "node1"};
        model = new NetworkModel("TestNetwork", nodes);

        // Setup initial state
        state.currentTime = 0;
        state.activeNodes = nodes;
    }

    void TearDown() override {
        delete model;
    }
};

// Test 1: Build the Network Atomic Model
TEST_F(NetworkAtomicFixture, testBuildNetworkAtomicModel) {
    EXPECT_EQ(model->input_ports.size(), 2);  // Should have 2 input ports
    EXPECT_EQ(model->output_ports.size(), 2); // Should have 2 output ports
    EXPECT_EQ(state.activeNodes.size(), 2);   // Active nodes are node0 and node1
}

// Test 2: Push Packets to Network Queue (via external transition)
TEST_F(NetworkAtomicFixture, testPushToNetworkQueue) {
    // Create a packet with RaftMessage and add it to input port "node0"
    std::shared_ptr<RaftMessage> raftMessage = std::make_shared<RaftMessage>();
    std::shared_ptr<Packet> packet = std::make_shared<Packet>(raftMessage, "node1", "node0");
    model->input_ports["node0"]->addMessage(packet);

    // Perform external transition
    model->externalTransition(state, 1.0);

    // Assert that the packet has been added to the packet queue
    EXPECT_EQ(state.packetQueue.size(), 1);
}

// Test 3: Output Transition - Forward Packet to Correct Output Port
TEST_F(NetworkAtomicFixture, testOutputTransition) {
    // Create a packet with RaftMessage and add it to input port "node0"
    std::shared_ptr<RaftMessage> raftMessage = std::make_shared<RaftMessage>();
    std::shared_ptr<Packet> packet = std::make_shared<Packet>(raftMessage, "node1", "node0");
    model->input_ports["node0"]->addMessage(packet);

    // Perform external transition
    model->externalTransition(state, 1.0);

    // Run the output function to forward the packet
    model->output(state);

    // Check that the packet is forwarded to output port "node1"
    EXPECT_EQ(model->output_ports["node1"]->getBag().size(), 1);
}

// Test 4: External Transition - Packet Queue Should Have 2 Events After Processing
TEST_F(NetworkAtomicFixture, testExternalTransition) {
    // Create packets with RaftMessages and add them to input ports
    std::shared_ptr<RaftMessage> raftMessage1 = std::make_shared<RaftMessage>();
    std::shared_ptr<RaftMessage> raftMessage2 = std::make_shared<RaftMessage>();

    std::shared_ptr<Packet> packet1 = std::make_shared<Packet>(raftMessage1, "*", "node0");
    std::shared_ptr<Packet> packet2 = std::make_shared<Packet>(raftMessage2, "*", "node1");

    model->input_ports["node0"]->addMessage(packet1);
    model->input_ports["node1"]->addMessage(packet2);

    // Perform external transition
    model->externalTransition(state, 1.0);

    // Assert that there are 2 events in the queue
    EXPECT_EQ(state.packetQueue.size(), 2);
}

// Test 5: Internal Transition - Ensure Packets Are Removed After Processing
TEST_F(NetworkAtomicFixture, testInternalTransition) {
    // Create a packet with RaftMessage and add it to the queue
    std::shared_ptr<RaftMessage> raftMessage = std::make_shared<RaftMessage>();
    std::shared_ptr<Packet> packet = std::make_shared<Packet>(raftMessage, "*", "node0");
    model->input_ports["node0"]->addMessage(packet);
    model->externalTransition(state, 1.0);

    // Perform internal transition
    model->internalTransition(state);

    // Assert that the queue is now empty
    EXPECT_EQ(state.packetQueue.size(), 0);
}

// Test 6: Time Advance With Events - Ensure Time is Advancing Correctly
TEST_F(NetworkAtomicFixture, testtimeAdvanceWithEvents) {
    // Create a packet with RaftMessage and add it to the queue
    std::shared_ptr<RaftMessage> raftMessage = std::make_shared<RaftMessage>();
    std::shared_ptr<Packet> packet = std::make_shared<Packet>(raftMessage, "*", "node0");
    model->input_ports["node0"]->addMessage(packet);
    model->externalTransition(state, 1.0);

    // Get the time advance (should not be infinity)
    double next_time = model->timeAdvance(state);
    
    // Assert that the time is a valid positive value
    EXPECT_TRUE(next_time > 0 && next_time != std::numeric_limits<double>::infinity());
}


// Main function for Google Test
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv); 
    return RUN_ALL_TESTS();
}