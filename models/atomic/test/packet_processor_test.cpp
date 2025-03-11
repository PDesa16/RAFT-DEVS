#include <gtest/gtest.h>
#include "../packet_processor.hpp"


class PacketProcessorAtomicFixture: public ::testing::Test
{
protected:
    std::unique_ptr<PacketProcessorModel> model;
    PacketProcessorState state{}; 

    void SetUp() override 
    {
        InitModel();
    }

    void InitModel() 
    {
        model.reset();
        model = std::make_unique<PacketProcessorModel>("packet-processor");
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
    
    
        model -> input_packet -> addMessage(packet1);
        model -> input_packet -> addMessage(packet2);
    };

};


// Test Model
TEST_F(PacketProcessorAtomicFixture, testBuildNetworkAtomicModel) {
    ASSERT_TRUE(model != nullptr);
}


// Main function for Google Test
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv); 
    return RUN_ALL_TESTS();
}