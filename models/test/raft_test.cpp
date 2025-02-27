#include <gtest/gtest.h>
#include "../raft.hpp"


TEST(TestRaftProtocol, TestRaftModelInit) {
    using MessageRaft = int;
    using MessageDatabase =  int;
    using PacketExternal = int;
    using  MessageConsensus = int;
    //Defining buffer state
    const RaftState raftState;
    // Creating buffer model
    RaftModel<MessageRaft, MessageDatabase, PacketExternal, MessageConsensus>* raftModel = new 
    RaftModel<MessageRaft, MessageDatabase, PacketExternal, MessageConsensus>("raftModel");
    raftModel -> timeAdvance(raftState);
    ASSERT_TRUE(1);
}

// Main function for Google Test
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv); 
    return RUN_ALL_TESTS();
}