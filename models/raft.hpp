#ifndef RAFT_PROTOCOL_HPP
#define RAFT_PROTOCOL_HPP

#include <cadmium/core/modeling/atomic.hpp>
#include <queue>
#include <string>
#include <vector>

using namespace cadmium;


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

// RequestVoteIn, AppendEntriesIn
// VoteResponseOut, AppendResponseOut

template <typename MessageRaft, typename MessageDatabase, typename PacketExternal, typename MessageConsensus>
class RaftModel : public Atomic<RaftState> {
public:

    Port<MessageRaft> input_buffer;
    Port<MessageDatabase> output_database;
    Port<MessageConsensus> output_consensus;
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
            if (msg > state.currentTerm) {
                state.currentTerm = msg;
                state.state = RaftStatus::FOLLOWER;
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

};



#endif