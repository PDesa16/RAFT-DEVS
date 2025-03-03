#include <gtest/gtest.h>
#include "../raft.hpp"
#include "../../utils/cryptography/crypto.hpp"


// struct MocklogEntryMetadata {
//     RequestVote ;
// };



class MockRaftAtomic  {
    public:
        // Store model & state
        RaftModel* model;
        RaftState state = RaftState();
        // Constructor default
        MockRaftAtomic() = default;
        // Init Test
        void InitModel() {
            model = new RaftModel("node0");
            // Create private key 
            std::string privateKey = Crypto::PrivateKeyToBase64(Crypto::GeneratePrivateKey());
            // Defining buffer state
            state.privateKey = privateKey;
            // Adding two "peers" to test logic
            state.peers = {
                "node1",
                "node2"
            };
        };


        // Mock Raft Input Message
        std::shared_ptr<RaftMessage> MockRaftMessageAppendEntriesNewLeader() {
                // Create RequestVote metadata
                RequestMetadata requestMetadata{
                    1,      // termNumber
                    "node0", // candidateID
                    0       // lastLogIndex
                };
            
                RequestVote requestVote(requestMetadata, ""); // Empty message digest for now
            
                // Create ResponseVotes
                ResponseMetadata responseMetadata1{
                    1,       // termNumber
                    "node0",  // votedFor
                    0,       // lastLogIndex
                    true,    // voteGranted
                    "node1"  // nodeId
                };
            
                ResponseMetadata responseMetadata2{
                    1,       // termNumber
                    "node0",  // votedFor
                    0,       // lastLogIndex
                    true,    // voteGranted
                    "node2"  // nodeId
                };
            
                std::vector<ResponseVote> responseVotes{
                    ResponseVote(responseMetadata1, ""),
                    ResponseVote(responseMetadata2, "")
                };
            
                // Create LogEntry metadata
                logEntryMetadata entryMetadata{
                    requestVote,
                    responseVotes
                };
            
                // Create LogEntryRAFT instance 
                auto logEntry = std::make_shared<LogEntryRAFT>(entryMetadata);

                 // Create a vector of log entries and pass it in a single step
                std::vector<std::shared_ptr<BaseMessageContentInterface<LogEntryType>>> logEntries {logEntry};
            
                // Create the AppendEntries metadata (using shared_ptr in the vector)
                AppendEntriesMetadata appendEntriesMetadata{
                    1,       // Term Number
                    "node0", // Leader ID
                    1,       // Index of log entry preceding the new entries
                    0,       // Term of the log entry at PrevLogIndex
                    logEntries,     // List of log entries to be replicated (empty for heartbeat)
                    0        // The index of the highest log entry known to be committed
                };
            
                // Create the AppendEntries object (pass shared_ptr)
                std::shared_ptr<AppendEntries> appendEntries = std::make_shared<AppendEntries>(appendEntriesMetadata, "");
            
                // Return the constructed RaftMessage (using shared_ptr)
                return std::make_shared<RaftMessage>(appendEntries); // Dereferencing here works with shared_ptr
            }
            
};


/* Protocol Init Tests */
TEST(TestRaftProtocol, TestRaftModelInit) {
    MockRaftAtomic raftMock;
    raftMock.InitModel();
    ASSERT_TRUE(raftMock.model != nullptr);
}


/* Internal Transition Tests */

TEST(TestRaftProtocol, TestInternalTransitionHeartbeatInvalid) {
    // Init Model
    MockRaftAtomic raftMock;
    raftMock.InitModel();
    // Step the internalTransition
    raftMock.model -> internalTransition(raftMock.state);
    // We expect the nodes to have no messages in the out queue as it hasnt timeout
    ASSERT_EQ(raftMock.state.raftOutMessages.size(), 0);

}

TEST(TestRaftProtocol, TestInternalTransitionHeartbeatValid) {
    // Init Model
    MockRaftAtomic raftMock;
    raftMock.InitModel();
    // set currentTime larger than 300ms
    raftMock.state.currentTime = 0.301;
    // Step the internalTransition
    raftMock.model -> internalTransition(raftMock.state);
    // We expect it to have created a VoteRequest Message
    ASSERT_EQ(Task::VOTE_REQUEST , raftMock.state.raftOutMessages[0]-> content -> getType());
    // We expect to have a message in our outboundRaft queue
    ASSERT_EQ(raftMock.state.raftOutMessages.size(), 1);
    // We expect the node to be a candidate
    ASSERT_EQ(raftMock.state.state, RaftStatus::CANDIDATE);
    // We expect our new timeout time to be larger than our current time
    ASSERT_GT(raftMock.state.heartbeatTimeout, raftMock.state.currentTime);
}

/* External Transition Tests */


/* Test ResponseVote */

TEST(TestRaftProtocol, TestHandleResponse) {
    // Init the model
    MockRaftAtomic raftMock;
    raftMock.InitModel();
    // We are a candidate, we receive a response that's valid. We should expect to store it in our temp storage.
    struct ResponseMetadata metadata {
        1,
        "node0",
        0,
        true,
        "node1"
        };
    // Create the expected message 
    std::shared_ptr<ResponseVote> responseVoteMessage =  std::make_shared<ResponseVote>(metadata, "");
    // Invoke Method
    raftMock.model -> HandleResponse(raftMock.state, responseVoteMessage);
    //Verify temp storage includes this entry. 
    ASSERT_EQ(raftMock.state.tempMessageStorage.size(), 1);
}

/* Test RequestVote */

TEST(TestRaftProtocol, TestHandleRequest) {
    // Init the model
    MockRaftAtomic raftMock;
    raftMock.InitModel();
    // We are a candidate, we receive a response that's valid. We should expect to store it in our temp storage.
    struct RequestMetadata metadata {
        1,
        "node1",
        0
        };
    // Create the expected message 
    std::shared_ptr<RequestVote> requestVoteMessage =  std::make_shared<RequestVote>(metadata, "");
    // Invoke Method
    raftMock.model -> HandleRequest(raftMock.state, requestVoteMessage);
    // Verify raftOutMessages includes this entry. 
    ASSERT_EQ(raftMock.state.raftOutMessages.size(), 1);
    // Verify that the message type is response
    ASSERT_EQ(raftMock.state.raftOutMessages.front() -> content -> getType(), Task::VOTE_RESPONSE);
}



/* Test AppendEntries */

TEST(TestRaftProtocol, TestHandleRAFTEntry) {
    // Init the model
    MockRaftAtomic raftMock;
    raftMock.InitModel();
    // Create the expected message 
    std::shared_ptr<RaftMessage> newLeaderMessage =  raftMock.MockRaftMessageAppendEntriesNewLeader();
    // Set the state 
    auto appendEntries = std::static_pointer_cast<AppendEntries>(newLeaderMessage -> content);
    auto raftEntry = std::static_pointer_cast<LogEntryRAFT>(appendEntries-> metadata.entries[0]);
    // Verify that our messageList has two entries
    ASSERT_EQ(raftEntry -> metadata.messageList.size(), 2);
    // Test RAFT Entry
    // Set node 0 as leader
    raftMock.state.leaderID = "node0";
    raftMock.model->HandleRAFTEntry(raftMock.state, raftEntry, "node0");
    //Verify messageLog includes this entry. If it does, then the log was commited.
    ASSERT_EQ(raftMock.state.messageLog.size(), 1);
}

TEST(TestRaftProtocol, TestHandleHeartbeatEntry) {
    // Init the model
    MockRaftAtomic raftMock;
    raftMock.InitModel();
    // Create the expected message 
    std::shared_ptr<LogEntryHeartbeat> logHeartBeatEntry =  std::make_shared<LogEntryHeartbeat>();
    // Set the state 
    raftMock.state.leaderID = "node1";
    // Run method
    raftMock.model->HandleHeartbeatEntry(raftMock.state, logHeartBeatEntry, "node1");
    // We expect to have a message in our message log
    ASSERT_EQ(raftMock.state.messageLog.size(), 1);
    // We expect our new timeout time to be larger than our current time
    ASSERT_GT(raftMock.state.heartbeatTimeout, raftMock.state.currentTime);
}

/* Test Time advance */

TEST(TestRaftProtocol, TestTimeAdvance) {
    // Init the model
    MockRaftAtomic raftMock;
    raftMock.InitModel();
    // Create the expected message 
    std::shared_ptr<LogEntryHeartbeat> logHeartBeatEntry =  std::make_shared<LogEntryHeartbeat>();
    // Set the state 
    raftMock.state.leaderID = "node1";
    // Run method
    raftMock.model->HandleHeartbeatEntry(raftMock.state, logHeartBeatEntry, "node1");
    // We expect to have a message in our message log
    ASSERT_EQ(raftMock.state.messageLog.size(), 1);
    // We expect our new timeout time to be larger than our current time
    ASSERT_GT(raftMock.state.heartbeatTimeout, raftMock.state.currentTime);
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
                switch (msg -> content->getType()) {
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


TEST(RaftSystemTest, TimeAdvanceCalculatesCorrectly) {
    // Init the model
    MockRaftAtomic raftMock;
    raftMock.InitModel();
    // Mock Method Deterministic
    RaftSystem raftSystem;
    // Create Messages Directly
    auto appendEntriesMsg = std::make_shared<AppendEntries>();
    auto requestVoteMsg = std::make_shared<RequestVote>();
    auto responseVoteMsg = std::make_shared<ResponseVote>();

    auto raftMsg1 = std::make_shared<RaftMessage>(appendEntriesMsg);
    auto raftMsg2 = std::make_shared<RaftMessage>(requestVoteMsg);
    auto raftMsg3 = std::make_shared<RaftMessage>(responseVoteMsg);


    raftMock.state.raftOutMessages.emplace_back(raftMsg1);
    raftMock.state.raftOutMessages.emplace_back(raftMsg2);
    raftMock.state.raftOutMessages.emplace_back(raftMsg3);

    // Expected: 2.0 + 1.5 + 1.0 = 4.5
    EXPECT_DOUBLE_EQ(raftSystem.timeAdvance(raftMock.state), 4.5);
}


/* Test Output */

TEST(RaftTest, OutputMethodProcessesMessagesCorrectly) {
    // Init the model
    MockRaftAtomic raftMock;
    raftMock.InitModel();

    // Create some test messages (assuming these are valid types)
    raftMock.state.raftOutMessages.emplace_back(std::make_shared<RaftMessage>(std::make_shared<AppendEntries>()));
    raftMock.state.raftOutMessages.emplace_back(std::make_shared<RaftMessage>(std::make_shared<RequestVote>()));
    raftMock.state.raftOutMessages.emplace_back(std::make_shared<RaftMessage>(std::make_shared<ResponseVote>()));

    // Call the output method
    raftMock.model -> output(raftMock.state);

    // Verify that messages were added correctly
    ASSERT_EQ(raftMock.model->output_database->size(), 0);
    ASSERT_EQ(raftMock.model->output_external->size(), 2);
}

// Main function for Google Test
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv); 
    return RUN_ALL_TESTS();
}