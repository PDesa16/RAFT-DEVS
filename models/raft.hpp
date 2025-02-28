#ifndef RAFT_PROTOCOL_HPP
#define RAFT_PROTOCOL_HPP

#include <cadmium/core/modeling/atomic.hpp>
#include "../messages/raft/raft_messages.hpp"
#include <queue>
#include <string>
#include <vector>
#include <unordered_map>
#include "../utils/cryptography/crypto.hpp"

using namespace cadmium;

using MessageRaft = MessageRaft;


enum class RaftStatus { FOLLOWER, CANDIDATE, LEADER };
enum class VoteStatus { VOTE_NOT_YET_SUBMITTED, VOTE_SUBMITTED };

struct RaftState {
    RaftStatus state = RaftStatus::FOLLOWER;
    int currentTerm = 0;
    VoteStatus votedFor = VoteStatus::VOTE_NOT_YET_SUBMITTED;
    int commitIndex = 0;
    int hearbeatTimeout = 0;
    double currentTime = 0;
    std::string voteRequestId;
    std::string privateKey;
    std::vector<std::string> publicKeys;
    std::unordered_map<int, std::unordered_map<std::string, std::vector<ResponseVote>>> messageLog;
    // Out bound message queues
    std::queue<DatabaseMessage> databaseOutMessages;
    std::queue<RaftMessage> raftOutMessages;

};

template <typename MessageRaft, typename MessageDatabase, typename MessageConsensus>
class RaftModel : public Atomic<RaftState> {
public:

    Port<MessageRaft> input_buffer;
    Port<MessageDatabase> output_database;
    Port<MessageRaft> output_external;

    RaftState state;

    RaftModel(const std::string& id) : Atomic<RaftState>(id, {}) {}

    void internalTransition(RaftState& s) const override { 
        // Check if we had a TIMEOUT by not receiving a HeartBEAT message
        if (state.currentTime >= state.hearbeatTimeout) {
            // Set ourselves as a candidate
            state.state = RaftStatus::CANDIDATE;
            state.currentTerm = state.currentTerm + 1;
            state.voteRequestId = "request-term-"+ std::to_string(state.currentTerm) + "-" + id
            state.heartbeatTimeout += state.currentTime + 0.1; // Add random generation 
            // Make a requestVote
            RequestMetadata requestMetadata = {
                requestMesssage.termNumber,
                id,
                state.commitIndex
            };
            // Hash + Sign it
            std::string msgDigestSigned = Crypto::SignData(requestMetadata.toString(), state.state.privateKey);
            // Append to response
            RequestVote requestMessage = {
                requestMetadata,
                msgDigestSigned
            };

            MessageRaft raftMessage = {
                requestMessage;
            }
            // Push it to the ouput port
            output_external -> addMessage(raftMessage);
        }
    }

    void externalTransition(RaftState& s, double e) const override {
        std::vector<MessageRaft> msgs_buffer = input_buffer -> getBag();
        for (auto& msg : msgs_buffer) {
                BaseMessageContentInterface msg = msgs_buffer;
                switch (msg.getType())
                {
                case Task::VOTE_REQUEST:
                    BaseMessageContentInterface outMsg;
                    HandleRequest(static_cast<RequestVote>(msg));
                    break;
                case Task::VOTE_RESPONSE:
                    HandleResponse(static_cast<ResponseVote>(msg));
                    break;
                case Task::APPEND_ENTRIES:
                    HandleAppendEntries(static_cast<AppendEntries>(msg));
                    break;
                default:
                    break;
                }
            } 
            
            // Check if we should transition to leader
            CheckAndTransitionToLeader();
    }
    

    void output(const RaftState& s) const override {
        if (state.state == RaftStatus::LEADER) {
            output_database -> addMessage(true);
        }
    }

    double timeAdvance(const RaftState& s) const override {
        return (state.state == RaftStatus::CANDIDATE) ? getProcessingDelay() : std::numeric_limits<double>::infinity();
    }
  
    double getProcessingDelay() const {
        return 0.05;
    }

    void HandleRequest(RequestVote requestMesssage) {
        bool largerThanCurrentTerm = (requestMessage.termNumber > state.currentTerm);
        bool equalButNotVoted = (requestMessage.termNumber == state.currentTerm) & (state.votedFor == VoteStatus::VOTE_NOT_YET_SUBMITTED);

        ResponseMetadata responseMetadata;

        if ( largerThanCurrentTerm || equalButNotVoted ) {
            // Send a valid RESPONSE, return true
            responseMetadata = {
                requestMesssage.termNumber,
                requestMesssage.candidateID,
                requestMesssage.lastLogIndex,
                true,
                id
            };
        } else {
            // Otherwise a node is a leader or candidate, return false
            responseMetadata = {
                requestMesssage.termNumber,
                requestMesssage.candidateID,
                requestMesssage.lastLogIndex,
                false,
                id
            };
        }

        // Hash + Sign it
        std::string msgDigestSigned = Crypto::SignData(responseMetadata.toString(), state.state.privateKey);
        // Append to response
        ResponseVote response = {
            responseMetadata,
            msgDigestSigned
        };

        // Create Raft Message
        MessageRaft raftMessage { response };
        // Push to output message queue
        raftOutMessages.push(raftMessage);

    };

    void HandleResponse(ResponseVote responseMessage){
        // Checks if response is a valid. 
        if (responseMessage.metadata.voteGrated == true) {
            state.messageLog[responseMessage.metadata.termNumber][responseMessage.metadata.voteRequestID].push_back(responseMessage);
        } 
    };

    void HandleAppendEntries(AppendEntriesMessage appendEntriesMessage){
        // Ensure that append entries came from the leader node 
        bool prevLogIndexMatches = responseMessage.prevLogIndex;  
        bool prevLogTermMatches = responseMessage.prevLogTerm;   
        bool prevLeaderCommitMatches = responseMessage.leaderCommit;  
        bool leaderIDMatches =  appendEntriesMessage.leaderID == state.leaderID;
        if (prevLogIndexMatches && prevLogTermMatches && prevLeaderCommitMatches && leaderIDMatches) {
            // Check type
            // If the type is log type
                // Check if its a Insert or Query
                    // Build DatabaseMessage
                    // Marshal the content to JSON
                    // Bind it to DatabaseMessage
                    // If Insert 
                        // Add type Insert
                    // Else 
                        // Add type Query
                    // Push to output port
            output_database -> addMessage()
        }
    };

    void CheckAndTransitionToLeader(){
         // If the node is a candidate, check if it has enough votes
        if (state.state == RaftStatus::CANDIDATE) {
            // Calculate the required number of votes (2f+1)
            int voteCountRequirement = (state.numOfPeers + 1) / 2;  // 2f+1
            int votesReceived = state.messageLog[state.currentTerm][state.voteRequestId].size();

            // If enough votes have been received, transition to the leader
            if (votesReceived >= voteCountRequirement) {
                state.state = RaftStatus::LEADER;
                // Perform actions required for the leader state (send AppendEntries, heartbeat, etc.)
                BaseMessageContentInterface<LogEntryType> entriesVector;
                LogEntryRAFT raftEntry = {
                    
                };
                LogEntryHeartbeat heartbeatEntry = {
                    
                };
                SendAppendEntries();  
            }
        }
    }

    void SendAppendEntries(BaseMessageContentInterface<LogEntryType> entries) {
        AppendEntriesMetadata appendEntriesMetadata = {
            state.currentTerm,
            id, 
            state.prevLogIndex; // required for node missing logs
            state.prevLogTerm;   // Term of the log entry at PrevLogIndex
            entries, // List of log entries to be replicated (empty for heartbeat)
            state.commitIndex  // The index of the highest log entry known to be committed
        };

        // Hash + Sign it
        std::string msgDigestSigned = Crypto::SignData(appendEntriesMetadata.toString(), state.state.privateKey);
        // Append to response
        AppendEntries appendEntriesMsg = {
            appendEntriesMetadata,
            msgDigestSigned
        };
        // Create Raft Message
        MessageRaft raftMessage { appendEntriesMsg };
        // Push to output message queue
        raftOutMessages.push(raftMessage);
    };

    void SendHeartbeat(){

    };


};



#endif