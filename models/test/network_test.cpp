#include <gtest/gtest.h>
#include "../network.hpp"
#include "../../messages/raft/raft_messages.hpp"


class NetworkAtomicFixture: public ::testing::Test
{
protected:
    std::unique_ptr<NetworkModel> model;
    NetworkState state{}; 

    void SetUp() override 
    {
        InitModel();
    }

    void InitModel() 
    {
        model.reset();
        model = std::make_unique<NetworkModel>("network");
    }

    void mockInputEvent() {
        std::shared_ptr<RaftMessage> raftMessage1 = std::make_shared<RaftMessage>();
        std::shared_ptr<RaftMessage> raftMessage2 = std::make_shared<RaftMessage>();
        std::shared_ptr<Packet> packet1 = std::make_shared<Packet>(
            raftMessage1,
            "node0",
            "node1"
        ) ;
    
        std::shared_ptr<Packet> packet2 = std::make_shared<Packet>(
            raftMessage2,
            "node0",
            "node1"
        ) ;
    
    
        model -> in_packet -> addMessage(packet1);
        model -> in_packet -> addMessage(packet2);
    };

};


// Test Model
TEST_F(NetworkAtomicFixture, testBuildNetworkAtomicModel) {
    ASSERT_TRUE(model != nullptr);
}

// Test Queue
TEST_F(NetworkAtomicFixture, testPushToNetworkQueue) {
    
    std::shared_ptr<RaftMessage> raftMessage = std::make_shared<RaftMessage>();
    std::shared_ptr<Packet> packet = std::make_shared<Packet>(
        raftMessage,
        "node0",
        "node1"
    ) ;

    std::shared_ptr<PacketEvent> packetEvent1 = std::make_shared<PacketEvent> (
        packet,
        0.01,
        0
    ) ;

    std::shared_ptr<PacketEvent> packetEvent2 = std::make_shared<PacketEvent> (
        packet,
        0.02,
        0
    ) ;

    std::shared_ptr<PacketEvent> packetEvent3 = std::make_shared<PacketEvent> (
        packet,
        0.02,
        1
    ) ;  

    // Push Event 2
    state.packetQueue.push(packetEvent2);
    // Push Event 3
    state.packetQueue.push(packetEvent3);
    // Push Event 1
    state.packetQueue.push(packetEvent1);

    // ueue should be sorted. When we perform the three pop operations.
    ASSERT_EQ(packetEvent1, state.packetQueue.top());
    state.packetQueue.pop();
    ASSERT_EQ(packetEvent2, state.packetQueue.top());
    state.packetQueue.pop();
    ASSERT_EQ(packetEvent3, state.packetQueue.top());
    

}

/* DEVS TEST */

// Test Output 
TEST_F(NetworkAtomicFixture, testOutputTransition) {
    mockInputEvent();
    // Run through an external
    model -> externalTransition(state, 0);
    // Run through an Ouput 
    model -> output(state);
    // Verify that the output port has only 1 item and that its the smallest
    auto packetListOut = model -> out_packet -> getBag();
    ASSERT_EQ(packetListOut.size(), 1);

}

// Test External
TEST_F(NetworkAtomicFixture, testExternalTransition) {
    mockInputEvent();

    // Test external
    model -> externalTransition(state, 0);

    // We expect the queue to have two new messages
    ASSERT_EQ(state.packetQueue.size(), 2);

}
// Test Internal 
TEST_F(NetworkAtomicFixture, testInternalTransition) {
    mockInputEvent();
    // run external to populate packQueue
    model -> externalTransition(state, 0);
    // run internal
    model -> internalTransition(state);
    // Verify that the queue has one less item
    ASSERT_EQ(state.packetQueue.size(), 1);

}
// Test TA
TEST_F(NetworkAtomicFixture, testtimeAdvanceWithEvents) {
    mockInputEvent();
    // run external to populate packQueue
    model -> externalTransition(state, 0);
    // Time advance
    auto next_time = model -> timeAdvance(state);
    // time Advance should not be 0 and should not be inf 
    ASSERT_TRUE(next_time != 0 && next_time != std::numeric_limits<double>::infinity());
}


// Main function for Google Test
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv); 
    return RUN_ALL_TESTS();
}