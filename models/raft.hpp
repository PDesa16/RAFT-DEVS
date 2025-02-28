#ifndef RAFT_PROTOCOL_HPP
#define RAFT_PROTOCOL_HPP

#include <cadmium/core/modeling/atomic.hpp>
#include "../messages/raft/raft_messages.hpp"
#include <queue>
#include <string>
#include <vector>

using namespace cadmium;

using MessageRaft = MessageRaft;


enum class RaftStatus { FOLLOWER, CANDIDATE, LEADER };

struct RaftState {
    RaftStatus state = RaftStatus::FOLLOWER;
    int currentTerm = 0;
    int votedFor = -1;
    std::vector<std::string> log;
    int commitIndex = 0;
    int lastApplied = 0;
    int votesReceived = 0;
};

// RAFT appends application logic to APPEND_ENTRIES

template <typename MessageRaft, typename MessageDatabase, typename PacketExternal, typename MessageConsensus>
class RaftModel : public Atomic<RaftState> {
public:

    Port<MessageRaft> input_buffer;
    Port<MessageDatabase> output_database;
    Port<PacketExternal> output_packet_external;

    RaftState state;

    RaftModel(const std::string& id) : Atomic<RaftState>(id, {}) {}

    void internalTransition(RaftState& s) const override {
        if (state.state == RaftStatus::CANDIDATE) {
            state.votesReceived = 1; // Vote for self
            state.currentTerm = state.currentTerm + 1;
        }
    }

    void externalTransition(RaftState& s, double e) const override {
        std::vector<MessageRaft> msgs_buffer = input_buffer -> getBag();
        for (auto& msg : msgs_buffer) {
                BaseMessageContentInterface msg = msgs_buffer;
                switch (msg.getType())
                {
                case Task::VOTE_REQUEST:
                    HandleRequest(static_cast<RequestVoteMessage>(msg));
                    break;
                case Task::VOTE_RESPONSE:
                    HandleResponse(static_cast<ResponseVoteMessage>(msg));
                    break;
                case Task::APPEND_ENTRIES:
                    HandleAppendEntries(static_cast<AppendEntriesMessage>(msg));
                    break;
                default:
                    break;
                }
            }  
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

    void HandleRequest(RequestVoteMessage requestMessage) {
        // If node is a follower 
            // Send a valid RESPONSE
        // Otherwise a node is a leader or candidate, return  
    };

    void HandleResponse(ResponseVoteMessage responseMessage){
        // If response is a valid. 
        // Increment count and store message in log
        // If count @ request_id == 2f+1 
        // Become Leader node. 
        // Otherwise, return
    };

    void HandleAppendEntries(AppendEntriesMessage appendEntriesMessage){
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
    };
};



#endif