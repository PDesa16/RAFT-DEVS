#include <gtest/gtest.h>
#include "../raft.hpp"
#include "../../utils/cryptography/crypto.hpp"


class MockRaftAtomic  {
    public:
        // Define Message Types
        using MessageRaft = RaftMessage;
        using MessageDatabase = DatabaseMessage;
        using MessageConsensus = int;
        // Store model & state
        RaftModel<MessageRaft, MessageDatabase, MessageConsensus>* model;
        RaftState state {};
        // Constructor default
        MockRaftAtomic() = default;
        // Init Test
        void InitModel() {
            model = new RaftModel<MessageRaft, MessageDatabase, MessageConsensus>("raftModel");
              // Create private key 
            std::string privateKey = Crypto::PrivateKeyToBase64(Crypto::GeneratePrivateKey());
            //Defining buffer state
            state.privateKey = privateKey;
        };
};

TEST(TestRaftProtocol, TestRaftModelInit) {
    MockRaftAtomic raftMock;
    raftMock.InitModel();
    ASSERT_TRUE(raftMock.model != nullptr);
}


TEST(TestRaftProtocol, TestInternalTransition) {
    // Init Model
    MockRaftAtomic raftMock;
    raftMock.InitModel();
    // Step the internalTransition
    raftMock.model -> internalTransition(raftMock.state);
    // We expect the nodes to have no messages in the out queue as it hasnt timeout
    ASSERT_EQ(raftMock.state.raftOutMessages.size(), 0);
    // Time advance to the timeout stage
    raftMock.model -> timeAdvance(raftMock.state);
    // We expect it to have created a VoteRequest Message
    ASSERT_EQ(Task::VOTE_REQUEST , raftMock.state.raftOutMessages.top().content.getType());
    // We expect to have a message in our outboundRaft queue
    ASSERT_EQ(raftMock.state.raftOutMessages.size(), 1);
}



// Main function for Google Test
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv); 
    return RUN_ALL_TESTS();
}