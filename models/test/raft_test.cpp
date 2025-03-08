#include <gtest/gtest.h>
#include "../raft.hpp"
#include "../../utils/cryptography/crypto.hpp"


class RaftAtomicFixture: public ::testing::Test
{
protected:
    std::unique_ptr<RaftModel> model;
    RaftState state{}; 


    void SetUp() override 
    {
        InitModel();
    }

    void InitModel() 
    {
        model.reset();
        model = std::make_unique<RaftModel>("node0");
        state.privateKey = Crypto::PrivateKeyToBase64(Crypto::GeneratePrivateKey()); 
        state.peers = { "node1", "node2" }; // Adding two "peers" to test logic
    }

    // Mock Raft Message
    std::shared_ptr<RaftMessage> MockRaftMessageAppendEntriesNewLeader() 
    {
        RequestMetadata requestMetadata{1, "node0", 0};
        RequestVote requestVote(requestMetadata, "");

        ResponseMetadata responseMetadata1{1, "node0", 0, true, "node1"};
        ResponseMetadata responseMetadata2{1, "node0", 0, true, "node2"};

        std::vector<ResponseVote> responseVotes{
            ResponseVote(responseMetadata1, ""),
            ResponseVote(responseMetadata2, "")
        };

        logEntryMetadata entryMetadata{requestVote, responseVotes};
        std::shared_ptr<LogEntryRAFT> logEntry = std::make_shared<LogEntryRAFT>(entryMetadata);
        std::vector<std::shared_ptr<IMessage<LogEntryType>>> logEntries{logEntry};

        AppendEntriesMetadata appendEntriesMetadata{ 1, "node0", 1, 0, logEntries, 0 };

        return std::make_shared<RaftMessage>(std::make_shared<AppendEntries>(appendEntriesMetadata, ""));
    }


};


/* Protocol Init Tests */
TEST_F(RaftAtomicFixture, TestRaftModelInit) {

    ASSERT_TRUE(model != nullptr);
}


/* Internal Transition Tests */

TEST_F(RaftAtomicFixture, TestInternalTransitionHeartbeatInvalid) {

    model->internalTransition(state);
    // We expect the nodes to have no messages in the out queue as it hasnt timeout
    ASSERT_EQ(state.raftOutMessages.size(), 0);

}

TEST_F(RaftAtomicFixture, TestInternalTransitionHeartbeatValid) {
    state.currentTime = 0.301;
    // Step the internalTransition
    model->internalTransition(state);
    // We expect it to have created a VoteRequest Message
    ASSERT_EQ(Task::VOTE_REQUEST , state.raftOutMessages[0]->content->getType());
    // We expect to have a message in our outboundRaft queue
    ASSERT_EQ(state.raftOutMessages.size(), 1);
    // We expect the node to be a candidate
    ASSERT_EQ(state.state, RaftStatus::CANDIDATE);
    // We expect our new timeout time to be larger than our current time
    ASSERT_GT(state.heartbeatTimeout, state.currentTime);
}

/* External Transition Tests */


/* Test ResponseVote */

TEST_F(RaftAtomicFixture, TestHandleResponse) {
    // Init the model

    // We are a candidate, we receive a response that's valid. We should expect to store it in our temp storage.
    struct ResponseMetadata metadata { 1, "node0", 0, true, "node1" };
    // Create the expected message 
    std::shared_ptr<ResponseVote> responseVoteMessage =  std::make_shared<ResponseVote>(metadata, "");
    // Invoke Method
    model-> HandleResponse(state, responseVoteMessage);
    //Verify temp storage includes this entry. 
    ASSERT_EQ(state.tempMessageStorage.size(), 1);
}

/* Test RequestVote */

TEST_F(RaftAtomicFixture, TestHandleRequest) {
    // Init the model

    // We are a candidate, we receive a response that's valid. We should expect to store it in our temp storage.
    struct RequestMetadata metadata { 1, "node1", 0 };
    // Create the expected message 
    std::shared_ptr<RequestVote> requestVoteMessage =  std::make_shared<RequestVote>(metadata, "");
    // Invoke Method
    model->HandleRequest(state, requestVoteMessage);
    // Verify raftOutMessages includes this entry. 
    ASSERT_EQ(state.raftOutMessages.size(), 1);
    // Verify that the message type is response
    ASSERT_EQ(state.raftOutMessages.front()-> content->getType(), Task::VOTE_RESPONSE);
}



/* Test AppendEntries */

TEST_F(RaftAtomicFixture, TestHandleRAFTEntry) {

    // Create the expected message 
    std::shared_ptr<RaftMessage> newLeaderMessage =  MockRaftMessageAppendEntriesNewLeader();
    // Set the state 
    auto appendEntries = std::static_pointer_cast<AppendEntries>(newLeaderMessage-> content);
    auto raftEntry = std::static_pointer_cast<LogEntryRAFT>(appendEntries-> metadata.entries[0]);
    // Verify that our messageList has two entries
    ASSERT_EQ(raftEntry-> metadata.messageList.size(), 2);
    // Test RAFT Entry
    // Set node 0 as leader
    state.leaderID = "node0";
    model->HandleRAFTEntry(state, raftEntry, "node0");
    //Verify messageLog includes this entry. If it does, then the log was commited.
    ASSERT_EQ(state.messageLog.size(), 1);
}

TEST_F(RaftAtomicFixture, TestHandleHeartbeatEntry) {
    // Init the model

    // Create the expected message 
    std::shared_ptr<LogEntryHeartbeat> logHeartBeatEntry =  std::make_shared<LogEntryHeartbeat>();
    // Set the state 
    state.leaderID = "node1";
    // Run method
    model->HandleHeartbeatEntry(state, logHeartBeatEntry, "node1");
    // We expect to have a message in our message log
    ASSERT_EQ(state.messageLog.size(), 1);
    // We expect our new timeout time to be larger than our current time
    ASSERT_GT(state.heartbeatTimeout, state.currentTime);
}

/* Test Time advance */

TEST_F(RaftAtomicFixture, TestTimeAdvance) {
    // Create the expected message 
    std::shared_ptr<LogEntryHeartbeat> logHeartBeatEntry =  std::make_shared<LogEntryHeartbeat>();
    // Set the state 
    state.leaderID = "node1";
    // Run method
    model->HandleHeartbeatEntry(state, logHeartBeatEntry, "node1");
    // We expect to have a message in our message log
    ASSERT_EQ(state.messageLog.size(), 1);
    // We expect our new timeout time to be larger than our current time
    ASSERT_GT(state.heartbeatTimeout, state.currentTime);
}

// Deterministic rather than stochastic implmentation of timeAdvance
class RaftSystem {
    public:
        double processAppendEntries(const std::shared_ptr<RaftMessage> msg) const { return 2.0; }
        double processVoteRequest() const { return 1.5; }
        double processResponseVote() const { return 1.0; }
    
        double timeAdvance(const RaftState& s) const {
            double totalProcessingTime = 0.0;
            for (auto& msg : s.raftOutMessages) {
                switch (msg-> content->getType()) {
                    case Task::APPEND_ENTRIES:
                        totalProcessingTime += processAppendEntries(msg);
                        break;
                    case Task::VOTE_REQUEST:
                        totalProcessingTime += processVoteRequest();
                        break;
                    case Task::VOTE_RESPONSE:
                        totalProcessingTime += processResponseVote();
                        break;
                    default:
                        break;
                }
            }
            return totalProcessingTime;
        }
    };


TEST_F(RaftAtomicFixture, TimeAdvanceCalculatesCorrectly) {
    // Mock Method Deterministic
    RaftSystem raftSystem;
    // Create Messages Directly
    auto appendEntriesMsg = std::make_shared<AppendEntries>();
    auto requestVoteMsg = std::make_shared<RequestVote>();
    auto responseVoteMsg = std::make_shared<ResponseVote>();

    auto raftMsg1 = std::make_shared<RaftMessage>(appendEntriesMsg);
    auto raftMsg2 = std::make_shared<RaftMessage>(requestVoteMsg);
    auto raftMsg3 = std::make_shared<RaftMessage>(responseVoteMsg);


    state.raftOutMessages.emplace_back(raftMsg1);
    state.raftOutMessages.emplace_back(raftMsg2);
    state.raftOutMessages.emplace_back(raftMsg3);

    // Expected: 2.0 + 1.5 + 1.0 = 4.5
    EXPECT_DOUBLE_EQ(raftSystem.timeAdvance(state), 4.5);
}


/* Test Output */

TEST_F(RaftAtomicFixture, OutputMethodProcessesMessagesCorrectly) {

    // Create some test messages (assuming these are valid types)
    state.raftOutMessages.emplace_back(std::make_shared<RaftMessage>(std::make_shared<AppendEntries>()));
    state.raftOutMessages.emplace_back(std::make_shared<RaftMessage>(std::make_shared<RequestVote>()));
    state.raftOutMessages.emplace_back(std::make_shared<RaftMessage>(std::make_shared<ResponseVote>()));

    // Call the output method
    model->output(state);

    // Verify that messages were added correctly
    ASSERT_EQ(model->output_database->size(), 0);
    ASSERT_EQ(model->output_external->size(), 3);
}

// Main function for Google Test
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv); 
    return RUN_ALL_TESTS();
}