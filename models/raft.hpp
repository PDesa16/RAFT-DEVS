#ifndef RAFT_PROTOCOL_HPP
#define RAFT_PROTOCOL_HPP

#include <cadmium/core/modeling/atomic.hpp>
#include "../messages/raft/raft_messages.hpp"
#include <queue>
#include <string>
#include <vector>
#include <unordered_map>
#include "../utils/cryptography/crypto.hpp"
#include "../messages/database/database_messages.hpp"
#include "../utils/stochastic/random.hpp"

using namespace cadmium;

enum class RaftStatus { FOLLOWER, CANDIDATE, LEADER };
enum class VoteStatus { VOTE_NOT_YET_SUBMITTED, VOTE_SUBMITTED };


// Define the output stream operator for RaftStatus
std::ostream& operator<<(std::ostream& os, RaftStatus status) {
    switch (status) {
        case RaftStatus::FOLLOWER:
            os << "FOLLOWER";
            break;
        case RaftStatus::CANDIDATE:
            os << "CANDIDATE";
            break;
        case RaftStatus::LEADER:
            os << "LEADER";
            break;
    }
    return os;
}

// Define the output stream operator for VoteStatus
std::ostream& operator<<(std::ostream& os, VoteStatus status) {
    switch (status) {
        case VoteStatus::VOTE_NOT_YET_SUBMITTED:
            os << "VOTE_NOT_YET_SUBMITTED";
            break;
        case VoteStatus::VOTE_SUBMITTED:
            os << "VOTE_GRANTED";
            break;
    }
    return os;
}

struct RaftState {
    RaftStatus state = RaftStatus::FOLLOWER;  // Current state of the node (FOLLOWER, CANDIDATE, LEADER)
    int currentTerm = 0;  // Current term of the node
    VoteStatus votedFor = VoteStatus::VOTE_NOT_YET_SUBMITTED;  // Vote status for the current term
    int commitIndex = 0;  // The index of the highest log entry known to be committed
    int lastApplied = 0;  // The index of the last applied entry (to the state machine)
    int heartbeatTimeout = 0;  // Timeout for heartbeat messages
    double currentTime = 0.0;  // Current time (used for heartbeat and election timeouts)
    std::string voteRequestId;  // Request ID for the current vote request
    std::string privateKey;  // Node's private key for signing messages
    std::vector<std::string> publicKeys;  // List of public keys of other nodes for signature verification
    std::unordered_map<int, std::unordered_map<std::string, std::vector<ResponseVote>>> messageLog;  // Log of responses from other nodes
    std::vector<DatabaseMessage> databaseOutMessages;  // Outgoing database messages (e.g., queries or inserts)
    std::vector<RaftMessage> raftOutMessages;  // Outgoing Raft messages (e.g., AppendEntries)
    int numOfPeers = 0;  // Total number of peers in the cluster (including this node)
    int logIndex = 0;  // Current index of the last log entry
    int electionTimeout = 0;  // Timeout for triggering a new election
    int votesReceived = 0;  // Number of votes received in the current election
    std::string leaderID;
};

// Define the output stream operator for RaftState
std::ostream& operator<<(std::ostream& os, const RaftState& state) {
    os << "RaftState { "
       << "state: " << state.state << ", "
       << "currentTerm: " << state.currentTerm << ", "
       << "votedFor: " << state.votedFor << ", "
       << "commitIndex: " << state.commitIndex << ", "
       << "heartbeatTimeout: " << state.heartbeatTimeout << ", "
       << "currentTime: " << state.currentTime << ", "
       << "voteRequestId: \"" << state.voteRequestId << "\", "
       << "privateKey: \"" << state.privateKey << "\", "
       << "publicKeys: [";
    
    for (size_t i = 0; i < state.publicKeys.size(); ++i) {
        os << "\"" << state.publicKeys[i] << "\"";
        if (i < state.publicKeys.size() - 1) os << ", ";
    }

    os << "], "
       << "numOfPeers: " << state.numOfPeers << ", "
       << "logIndex: " << state.logIndex
       << " }";
    
    return os;
}

template <typename MessageRaft, typename MessageDatabase, typename MessageConsensus>
class RaftModel : public Atomic<RaftState> {
public:

    Port<MessageRaft> input_buffer;
    Port<MessageDatabase> output_database;
    Port<MessageRaft> output_external;

    RaftState state {};

    RaftModel(const std::string& id) : Atomic<RaftState>(id, {}) {}

    void internalTransition(RaftState& s) const override { 
        // Check if we had a TIMEOUT by not receiving a HeartBEAT message
        if (s.currentTime >= s.heartbeatTimeout) {
            // Set ourselves as a candidate
            s.state = RaftStatus::CANDIDATE;
            s.currentTerm = state.currentTerm + 1;
            s.voteRequestId = "request-term-"+ std::to_string(s.currentTerm) + "-" + id;
            s.heartbeatTimeout = s.currentTime + 0.1; // Add random generation 
            // Make a requestVote
            RequestMetadata requestMetadata = {
                s.currentTerm,
                id,
                s.commitIndex
            };
            // Hash + Sign it
            std::string msgDigestSigned = Crypto::SignData(requestMetadata.toString(), s.privateKey);
            // Append to response
            RequestVote requestMessage = {
                requestMetadata,
                msgDigestSigned
            };

            MessageRaft raftMessage = {
                requestMessage
            };
            // Push it to the ouput port
            s.raftOutMessages.push(raftMessage);
        }
    }

    void externalTransition(RaftState& s, double e) const override {
        // Update time
        s.currentTime += e;

        std::vector<MessageRaft> msgs_buffer = input_buffer -> getBag();
        for (auto& msgRaft : msgs_buffer) {
                switch (msgRaft.content.getType())
                {
                case Task::VOTE_REQUEST:
                    HandleRequest(s, static_cast<RequestVote&>(msgRaft.content));
                    break;
                case Task::VOTE_RESPONSE:
                    HandleResponse(s, static_cast<ResponseVote&>(msgRaft.content));
                    break;
                case Task::APPEND_ENTRIES:
                    HandleAppendEntries(s, static_cast<AppendEntries&>(msgRaft.content));
                    break;
                default:
                    break;
                }
            } 
            
            // Check if we should transition to leader
            CheckAndTransitionToLeader(s);
    }
    

    void output(RaftState& s) const override {
        // Perform the Database messages first, Independent 
        while (!s.databaseOutMessages.empty()) {
            output_database->addMessage(s.databaseOutMessages.top());
            s.databaseOutMessages.pop();
        }
    
        // Perform the OutRaft messages second
        while (!s.raftOutMessages.empty()) {
            output_external->addMessage(s.raftOutMessages.top());
            s.raftOutMessages.pop();
        }
    }
    

    double timeAdvance(RaftState& s) const override {
        double totalProcessingTime = 0.0;  // Initialize the total processing time accumulator
    
        // Loop through raftOutMessages queue and calculate delays based on message types
        for (auto& msg : s.raftOutMessages) {
            switch (msg.getType()) {
                case Task::APPEND_ENTRIES:
                    totalProcessingTime += processAppendEntries(msg);
                    break;
                case Task::VOTE_REQUEST:
                    totalProcessingTime += processVoteRequest(msg);
                    break;
                case Task::VOTE_RESPONSE:
                    totalProcessingTime += processResponseVote(msg);
                    break;
                default:
                    break;
            }
        }

        // Add total delay to current time and return it
        s.currentTime += totalProcessingTime;
        return s.currentTime;
    }


void HandleRequest(RaftState& s, RequestVote& requestMessage) const {
    bool largerThanCurrentTerm = (requestMessage.metadata.termNumber > s.currentTerm);
    bool equalButNotVoted = (requestMessage.metadata.termNumber == s.currentTerm) && (s.votedFor == VoteStatus::VOTE_NOT_YET_SUBMITTED);

    ResponseMetadata responseMetadata;
    bool voteGranted = largerThanCurrentTerm || equalButNotVoted;

    responseMetadata = {
        requestMessage.metadata.termNumber,
        requestMessage.metadata.candidateID,
        requestMessage.metadata.lastLogIndex,
        voteGranted,
        id
    };

    // Hash + Sign it
    std::string msgDigestSigned = Crypto::SignData(responseMetadata.toString(), s.privateKey);
    
    // Append to response
    ResponseVote response = { responseMetadata, msgDigestSigned };

    // Create Raft Message
    MessageRaft raftMessage { response };

    // Push to output message queue
    s.raftOutMessages.push(raftMessage);
}

    void HandleResponse(RaftState& s, ResponseVote& responseMessage) const {
        // Checks if response is a valid. 
        if (responseMessage.metadata.voteGranted == true) {
            s.messageLog[responseMessage.metadata.termNumber][s.voteRequestId].push_back(responseMessage);
        } 
    };

    void HandleAppendEntries(RaftState& s, AppendEntries& appendEntriesMessage) const {
        // Ignore stale terms (leader must have a higher term)
        if (appendEntriesMessage.metadata.term < s.currentTerm) {
            return;
        }
    
        // Loop through the entries
        for (const auto& logEntry : appendEntriesMessage.metadata.entries) {
            // Update leader information if the term is valid
            switch (logEntry.getType()) {
                case LogEntryType::RAFT:
                    // Loop through the messages (verify them, set leader if valid, and commit to log)
                    HandleRAFTEntry(s, logEntry, appendEntriesMessage.metadata.leaderId);
                    break;
    
                case LogEntryType::HEARTBEAT:
                    // Verify leader is valid and update log with heartbeat metadata
                    HandleHeartbeat(s, appendEntriesMessage.metadata.leaderID);
                    break;
    
                case LogEntryType::EXTERNAL:
                    // Ignore for now, logic added later
                    break;
    
                default:
                    // Handle unrecognized log entry types (can log or take further action if needed)
                    break;
            }
        }
    
        // Remove conflicting log entries
        s.log.resize(appendEntriesMessage.metadata.prevLogIndex + 1);
    
        // Append new entries from the message
        s.log.insert(s.log.end(), appendEntriesMessage.metadata.entries.begin(), appendEntriesMessage.metadata.entries.end());
    
        // Update commit index: Ensure we don't commit unreplicated entries
        s.commitIndex = std::min(appendEntriesMessage.metadata.leaderCommit, static_cast<int>(s.log.size()) - 1);
    }
    
    void HandleRAFTEntry(RaftState& s, const std::shared_ptr<BaseMessageContentInterface<LogEntryType>>& logEntry, const std::string& leaderID) const {
        // Verify the RAFT entry before committing it
        if (ValidateRAFTEntry(logEntry)) {
            // If the entry is valid, commit to the log
            s.log.push_back(logEntry);  // Or handle it according to your log structure
    
            // Update the leader if the entry is valid and the leader has changed
            s.leaderID = leaderID;
        } else {
            // Handle invalid RAFT entry, e.g., log an error or take action
            std::cerr << "Invalid RAFT entry detected. Skipping commit." << std::endl;
        }
    }
    
    void HandleHeartbeat(RaftState& s, const std::string& leaderID) const {
        // Verify that the leader is valid
        if (s.leaderID != leaderID) {
            std::cerr << "Heartbeat received from an invalid leader: " << leaderID << std::endl;
            return;
        }
    
        // Commit the heartbeat metadata to the log 
        // Set new heartbeat time
        s.heartbeatTimeout = s.currentTime + RandomNumberGeneratorDEVS::generateUniformDelay(0.150,0.300);

    }
    
    bool ValidateRAFTEntry(const std::shared_ptr<BaseMessageContentInterface<LogEntryType>>& entry) const {
        // Perform validation checks for the RAFT entry (e.g., verify signature, content, etc.)
        // Assume is valid for this experimental frame
        bool isValid = true;
        return isValid;
    }
    
    
    
    void CheckAndTransitionToLeader(RaftState& s) const {
        // If the node is a candidate, check if it has enough votes
        if (s.state == RaftStatus::CANDIDATE) {
            // Calculate the required number of votes (2f+1), assuming s.numOfPeers includes itself
            int voteCountRequirement = (s.numOfPeers / 2) + 1;
            int votesReceived = s.messageLog[s.currentTerm][s.voteRequestId].size();
    
            // If enough votes have been received, transition to leader
            if (votesReceived >= voteCountRequirement) {
                s.state = RaftStatus::LEADER;
    
                // Prepare heartbeat log entries
                std::vector<std::shared_ptr<BaseMessageContentInterface<LogEntryType>>> entriesVector;
    
                // Heartbeat metadata
                HeartbeatMetadata metadataHeartbeat = {
                    id,                // Leader ID
                    0,                 // Sequence number (should be incremented if necessary)
                    s.currentTime,  // Timestamp (if required)
                    HEARTBEAT_STATUS::PING
                };
    
                // Create log entries
                entriesVector.push_back(std::make_shared<LogEntryHeartbeat>(metadataHeartbeat));
                entriesVector.push_back(std::make_shared<LogEntryRAFT>());
    
                // Send the AppendEntries message
                SendAppendEntries(s, entriesVector);  
            }
        }
    }
    
    void SendAppendEntries(RaftState& s, std::vector<std::shared_ptr<BaseMessageContentInterface<LogEntryType>>> entries) const {
        // Prepare append entries metadata
        AppendEntriesMetadata appendEntriesMetadata = {
            s.currentTerm,   // Leader's term
            id,              // Leader's ID
            s.logIndex,      // Index for the log entry at PrevLogIndex
            s.currentTerm,   // Term of the log entry at PrevLogIndex
            entries,         // Entries to replicate (including heartbeat)
            s.commitIndex    // The highest log entry index known to be committed
        };
    
        // Hash and sign the message
        std::string msgDigestSigned = Crypto::SignData(appendEntriesMetadata.toString(), s.privateKey);
    
        // Create append entries message with signature
        AppendEntries appendEntriesMsg = {
            appendEntriesMetadata,
            msgDigestSigned
        };
    
        // Create Raft message and add it to the output message queue
        MessageRaft raftMessage { appendEntriesMsg };
        s.raftOutMessages.push(raftMessage);
    }
    

};

// Function to calculate processing delay for AppendEntries messages
double processAppendEntries(const RaftMessage& msg) {
    size_t numEntries = static_cast<AppendEntries>(msg.content).metadata.entries.size();
    double lambda = 0.1;  // Lambda is the rate (1/mean), adjust as needed for your system
    double baseDelay = 0.5;  // Base processing time for each entry, you can fine-tune this
    return numEntries * RandomNumberGeneratorDEVS::generateExponentialDelay(lambda) * baseDelay;
}

// Function to calculate processing delay for VoteRequest messages
double processVoteRequest() {
    double lambda = 0.2;  // Lambda for vote request delays (tune to your needs)
    double baseDelay = 0.1;  // Base delay for vote requests
    return RandomNumberGeneratorDEVS::generateExponentialDelay(lambda) * baseDelay;
}

// Function to calculate processing delay for ResponseVote messages
double processResponseVote() {
    double lambda = 0.3;  // Lambda for response vote delays
    double baseDelay = 0.05;  // Base delay for response votes
    return RandomNumberGeneratorDEVS::generateExponentialDelay(lambda) * baseDelay;
}


#endif