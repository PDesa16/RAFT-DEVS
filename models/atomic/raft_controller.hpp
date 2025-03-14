#ifndef RAFT_CONTROLLER_HPP
#define RAFT_CONTROLLER_HPP

#include <cadmium/core/modeling/atomic.hpp>
#include "../../messages/raft/raft_messages.hpp"
#include <queue>
#include <string>
#include <vector>
#include <unordered_map>
#include "../../utils/cryptography/crypto.hpp"
#include "../../messages/database/database_messages.hpp"
#include "../../utils/stochastic/random.hpp"

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
    VoteStatus votedStatus = VoteStatus::VOTE_NOT_YET_SUBMITTED;  // Vote status for the current term
    HeartbeatStatus heartbeatStatus = HeartbeatStatus::ALIVE;
    int currentTerm = 0;  // Current term of the node
    int commitIndex = 0;  // The index of the highest log entry known to be committed
    int lastApplied = 0;  // The index of the last applied entry (to the state machine)
    double currentTime = 0.0;  // Current time (used for heartbeat and election timeouts)
    std::string privateKey;  // Node's private key for signing messages
    std::vector<std::string> publicKeys;  // List of public keys of other nodes for signature verification
    std::vector<std::shared_ptr<IMessage<LogEntryType>>> messageLog; // Log of responses from other nodes
    std::vector<std::shared_ptr<ResponseVote>> tempMessageStorage; 
    std::vector<std::shared_ptr<DatabaseMessage>> databaseOutMessages;  // Outgoing database messages (e.g., queries or inserts)
    std::vector<std::shared_ptr<RaftMessage>> raftOutMessages;  // Outgoing Raft messages (e.g., AppendEntries)
    std::vector<std::string> peers;  // Total number of peers in the cluster (including this node)
    int logIndex = 0;  // Current index of the last log entry
    int electionTimeout = 0;  // Timeout for triggering a new election
    double lastHeartbeatUpdate = 0 ;
    std::string nodeID;
    std::string leaderID;
    std::shared_ptr<RequestVote> leaderProof;
    

    friend std::ostream& operator<<(std::ostream& os, const RaftState& state) {
        os << "RaftState { "
           << "state: " << state.state << ", "
           << "currentTerm: " << state.currentTerm << ", "
           << "votedStatus: " << state.votedStatus << ", "
           << "commitIndex: " << state.commitIndex << ", "
           << "currentTime: " << state.currentTime << ", "
           << "privateKey: \"" << state.privateKey << "\", "
           << "publicKeys: [";
        
        for (size_t i = 0; i < state.publicKeys.size(); ++i) {
            os << "\"" << state.publicKeys[i] << "\"";
            if (i < state.publicKeys.size() - 1) os << ", ";
        }
    
        os << "], "
           << "numOfPeers: " << state.peers.size() << ", "
           << "logIndex: " << state.logIndex
           << " }";
        
        return os;
    }
};



class RaftControllerModel : public Atomic<RaftState> {
public:
    Port<std::shared_ptr<RaftMessage>> input_buffer;
    Port<std::shared_ptr<DatabaseMessage>> output_database;
    Port<std::shared_ptr<RaftMessage>> output_external;
    Port<HeartbeatStatus> output_heartbeat;
    Port<HeartbeatStatus> input_heartbeat;


    RaftControllerModel(const std::string& id)
    : Atomic<RaftState>(id, {}) {
        input_buffer = addInPort<std::shared_ptr<RaftMessage>>("input_buffer");
        input_heartbeat = addInPort<HeartbeatStatus>("input_heartbeat");
        output_database = addOutPort<std::shared_ptr<DatabaseMessage>>("output_database");
        output_external = addOutPort<std::shared_ptr<RaftMessage>>("output_external");
        output_heartbeat = addOutPort<HeartbeatStatus>("output_heartbeat");
        // output_heartbeat -> addMessage(HeartbeatStatus::ALIVE);
    }


    // Function to calculate processing delay for AppendEntries messages
    double processAppendEntries(const std::shared_ptr<RaftMessage> msg) const {
        size_t numEntries = std::static_pointer_cast<AppendEntries>(msg->content) -> metadata.entries.size();
        double lambda = 10000;  // Lambda is the rate (1/mean), adjust as needed for your system
        return numEntries * RandomNumberGeneratorDEVS::generateExponentialDelay(lambda);
    }

    // Function to calculate processing delay for VoteRequest messages
    double processVoteRequest() const {
        double lambda = 100000;  // Lambda for vote request delays (tune to your needs)
        return RandomNumberGeneratorDEVS::generateExponentialDelay(lambda);
    }

    // Function to calculate processing delay for ResponseVote messages
    double processResponseVote() const {
        double lambda = 100000;  // Lambda for response vote delays
        return RandomNumberGeneratorDEVS::generateExponentialDelay(lambda);
    }


    void internalTransition(RaftState& s) const override { 
        s.heartbeatStatus = HeartbeatStatus::ALIVE;
        // Flush message vectors
        s.databaseOutMessages.clear();
        s.raftOutMessages.clear();

    }

    void externalTransition(RaftState& s, double e) const override {
        // Update time
        s.currentTime += e;

        std::vector<std::shared_ptr<RaftMessage>> msgs_buffer = input_buffer -> getBag();
        for (auto& msgRaft : msgs_buffer) {
                switch (msgRaft -> content -> getType())
                {
                case Task::VOTE_REQUEST:
                    HandleRequest(s, std::static_pointer_cast<RequestVote>(msgRaft -> content), msgRaft -> source);
                    break;
                case Task::VOTE_RESPONSE:
                    HandleResponse(s, std::static_pointer_cast<ResponseVote>(msgRaft -> content));
                    break;
                case Task::APPEND_ENTRIES:
                    HandleAppendEntries(s, std::static_pointer_cast<AppendEntries>(msgRaft -> content));
                    break;
                default:
                    break;
                }
            } 


            // Check for HeartbeatEvents 
            HeartbeatStatus heartbeatStatus = input_heartbeat -> getBag().size() > 0 ? input_heartbeat -> getBag()[0] : HeartbeatStatus::ALIVE  ;
            CheckAndTransitionHeartbeat(s, heartbeatStatus);
            
            // Check if we should transition to leader
            CheckAndTransitionToLeader(s);
    }
    

    void output(const RaftState& s) const override {

        // Perform the Database messages first, Independent
        for (auto& message : s.databaseOutMessages) {
            output_database->addMessage(message);  
        }
    
        // Perform the OutRaft messages second
        for (auto& message : s.raftOutMessages) {
            output_external->addMessage(message);
        }

        if (s.heartbeatStatus == HeartbeatStatus::UPDATE) {
            output_heartbeat -> addMessage(HeartbeatStatus::UPDATE);
        }

        if (s.heartbeatStatus == HeartbeatStatus::TIMEOUT) {
            output_heartbeat -> addMessage(HeartbeatStatus::ALIVE);
        } 
    }


    double timeAdvance(const RaftState& s) const override {
        double totalProcessingTime = 0;  // Initialize the total processing time accumulator
    
        // Loop through raftOutMessages queue and calculate delays based on message types
        for (auto& msg : s.raftOutMessages) {
            switch (msg -> content -> getType()) {
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
        
        return totalProcessingTime == 0 ? std::numeric_limits<double>::infinity() : totalProcessingTime ;
    }



void HandleRequest(RaftState& s, std::shared_ptr<RequestVote> requestMessage, std::string source) const {
    bool largerThanCurrentTerm = (requestMessage -> metadata.termNumber > s.currentTerm);
    bool equalButNotVoted = (requestMessage -> metadata.termNumber == s.currentTerm) && (s.votedStatus == VoteStatus::VOTE_NOT_YET_SUBMITTED);

    ResponseMetadata responseMetadata;
    bool voteGranted = largerThanCurrentTerm || equalButNotVoted;

    responseMetadata = {
        requestMessage -> metadata.termNumber,
        requestMessage -> metadata.candidateID,
        requestMessage -> metadata.lastLogIndex,
        voteGranted,
        s.nodeID
    };

    // // Hash + Sign it
    // std::string msgDigestSigned = Crypto::SignData(responseMetadata.toString(), s.privateKey);
    
    // Append to response
    std::shared_ptr<ResponseVote> response = std::make_shared<ResponseVote>(responseMetadata, "msgDigestSigned");

    // Create Raft Message
    std::shared_ptr<RaftMessage> raftMessage =  std::make_shared<RaftMessage>(response);
    // Return to Requestor
    raftMessage -> dest = source;
    raftMessage -> source = s.nodeID;
    // Push to output message queue
    s.raftOutMessages.emplace_back(raftMessage);
}

    void HandleResponse(RaftState& s, std::shared_ptr<ResponseVote> responseMessage) const {
        // Checks if response is a valid. 
        if (responseMessage -> metadata.voteGranted == true) {
            s.tempMessageStorage.emplace_back(responseMessage);
        } 
    };

    void HandleAppendEntries(RaftState& s, std::shared_ptr<AppendEntries> appendEntriesMessage) const {
        // Ignore stale terms (leader must have a higher term)
        if (appendEntriesMessage -> metadata.term < s.currentTerm) {
            return;
        }
    
        // Loop through the entries
        for (auto& logEntry : appendEntriesMessage -> metadata.entries) {
            // Update leader information if the term is valid
            switch (logEntry -> getType()) {
                case LogEntryType::RAFT:
                    HandleRAFTEntry(s, std::static_pointer_cast<LogEntryRAFT>(logEntry), appendEntriesMessage -> metadata.leaderID);
                    break;
    
                case LogEntryType::HEARTBEAT:
                    // Verify leader is valid and update log with heartbeat metadata
                    HandleHeartbeatEntry(s, std::static_pointer_cast<LogEntryHeartbeat>(logEntry), appendEntriesMessage -> metadata.leaderID);
                    break;
    
                case LogEntryType::EXTERNAL:
                    // Ignore for now, logic added later
                    break;
    
                default:
                    // Handle unrecognized log entry types (can log or take further action if needed)
                    break;
            }
        }

        // Update commit index
        s.commitIndex = std::min(appendEntriesMessage -> metadata.leaderCommit, static_cast<int>(s.messageLog.size()) - 1);
    }
    
    void HandleRAFTEntry(RaftState& s, const std::shared_ptr<LogEntryRAFT> logEntryRaft, const std::string& leaderID) const {
        // Verify the RAFT entry before committing it
        if (ValidateRAFTEntry(s, logEntryRaft)) {
            // If the entry is valid, commit to the log
            s.messageLog.emplace_back(logEntryRaft);  // Or handle it according to your log structure
            std::cout << "Node #" << s.nodeID
            << " | Message Log Entry #" << s.messageLog.size()
            << " | Log Entry: " << logEntryRaft->toString()
            << std::endl;  
            // Update the leader if the entry is valid and the leader has changed
            s.leaderID = leaderID;
            s.lastHeartbeatUpdate = s.currentTime;
        } else {
            // Handle invalid RAFT entry, e.g., log an error or take action
            std::cerr << "Invalid RAFT entry detected. Skipping commit." << std::endl;
        }
    }
    
    void HandleHeartbeatEntry(RaftState& s, const std::shared_ptr<LogEntryHeartbeat> logEntryHeartbeat, const std::string& leaderID) const {
        // Verify that the leader is valid
        if (s.leaderID != leaderID) {
            // std::cerr << "Heartbeat received from an invalid leader: " << leaderID << std::endl;
            return;
        }
        // Commit message
        s.lastHeartbeatUpdate = s.currentTime;
        s.messageLog.emplace_back(logEntryHeartbeat);
        std::cout << "Node #" << s.nodeID 
          << " | Message Log Entry #" << s.messageLog.size() 
          << " | Log Entry: " << logEntryHeartbeat->toString() 
          << std::endl;

    }
    
    bool ValidateRAFTEntry(const RaftState& s, const std::shared_ptr<LogEntryRAFT> logEntryRaft) const {
        // Perform validation checks for the RAFT entry (e.g., verify signature, content, etc.)
        // Assume is valid for this experimental frame
        int voteCountRequirement = std::ceil((s.peers.size() + 1) / 2.0);

        // Validate signatures, from peers -- implement in later project
        // Validate request from leader
        int acc = 0;
        for (const auto& message : logEntryRaft -> metadata.messageList) {
            if (message.metadata.voteGranted == true){
                acc++;
            }
        }
        return acc >= voteCountRequirement;
    }
    
    
    void CheckAndTransitionToLeader(RaftState& s) const {
        // If the node is a candidate, check if it has enough votes
        if (s.state == RaftStatus::CANDIDATE) {
            // Calculate the required number of votes (2f+1)
            int voteCountRequirement = std::ceil((s.peers.size() + 1) / 2.0);

            int votesReceived = s.tempMessageStorage.size();
    
            // If enough votes have been received, transition to leader
            if (votesReceived >= voteCountRequirement) {
                s.state = RaftStatus::LEADER;
                s.leaderID = s.nodeID;

                s.heartbeatStatus = HeartbeatStatus::UPDATE;
    
                // Prepare heartbeat log entries
                std::vector<std::shared_ptr<IMessage<LogEntryType>>> entriesVector;
    
                // Heartbeat metadata
                HeartbeatMetadata metadataHeartbeat = {
                    s.nodeID,                // Leader ID
                    s.logIndex,       // Sequence number (should be incremented if necessary)
                    s.currentTime,  // Timestamp (if required)
                    HEARTBEAT_STATUS::PING
                };
    
                // Create log entries
                entriesVector.emplace_back(std::make_shared<LogEntryHeartbeat>(metadataHeartbeat));


                // Copy proof
                std::vector<ResponseVote> messageList;
                for (const auto& ptr : s.tempMessageStorage) {
                    messageList.push_back(*ptr);  // Dereference shared_ptr to copy the object
                }
                
                
                // Make a copy of Request
                RequestVote request = *s.leaderProof;

                // Create Log Entry
                logEntryMetadata LEM = {
                    request,
                    messageList
                };

                entriesVector.emplace_back(std::make_shared<LogEntryRAFT>(LEM));                              
    
                // Send the AppendEntries message
                SendAppendEntries(s, entriesVector);  
            }
        }
    }
    
    void SendAppendEntries(RaftState& s, std::vector<std::shared_ptr<IMessage<LogEntryType>>> entries) const {
        // Prepare append entries metadata
        AppendEntriesMetadata appendEntriesMetadata = {
            s.currentTerm,   // Leader's term
            s.nodeID,              // Leader's ID
            s.logIndex,      // Index for the log entry at PrevLogIndex
            s.currentTerm,   // Term of the log entry at PrevLogIndex
            entries,         // Entries to replicate (including heartbeat)
            s.commitIndex    // The highest log entry index known to be committed
        };
    
        // // Hash and sign the message
        // std::string msgDigestSigned = Crypto::SignData(appendEntriesMetadata.toString(), s.privateKey);
    
        // Create append entries message with signature
        std::shared_ptr<AppendEntries> appendEntriesMsg = std::make_shared<AppendEntries>(appendEntriesMetadata, "msgDigestSigned");

    
        // Create Raft message and add it to the output message queue
        std::shared_ptr<RaftMessage> raftMessage =  std::make_shared<RaftMessage>(appendEntriesMsg);
        raftMessage -> dest = "*";
        raftMessage -> source = s.nodeID;
        s.raftOutMessages.emplace_back(raftMessage);
    }

    void CheckAndTransitionHeartbeat(RaftState& s, HeartbeatStatus heartbeatStatus) const{

        // No Heartbeat Timeouts
        if (heartbeatStatus == HeartbeatStatus::ALIVE) {
            return;
        }
        

        // The leader node is the one driving heartbeat updates
        if (s.state == RaftStatus::LEADER && heartbeatStatus == HeartbeatStatus::UPDATE) {
            // Prepare heartbeat log entries
            std::vector<std::shared_ptr<IMessage<LogEntryType>>> entriesVector;

            s.lastHeartbeatUpdate = s.currentTime;

            // Heartbeat metadata
            HeartbeatMetadata metadataHeartbeat = {
                s.nodeID,          // Leader ID
                s.logIndex,       // Sequence number (should be incremented if necessary)
                s.currentTime,  // Timestamp (if required)
                HEARTBEAT_STATUS::PING
            };

            // Create log entries
            entriesVector.emplace_back(std::make_shared<LogEntryHeartbeat>(metadataHeartbeat));

            // Create and append to outbound queue
            SendAppendEntries(s, entriesVector);
            
            return;

        }
        
        // Check if we had a TIMEOUT by not receiving a HeartBEAT message
        if (s.state != RaftStatus::LEADER &&  heartbeatStatus == HeartbeatStatus::TIMEOUT && (s.currentTime - s.lastHeartbeatUpdate) > 0.150 ) {
                // Set ourselves as a candidate
                s.state = RaftStatus::CANDIDATE;
                s.currentTerm = state.currentTerm + 1;
                s.heartbeatStatus = HeartbeatStatus::TIMEOUT;

                s.votedStatus = VoteStatus::VOTE_SUBMITTED; // Self vote
                // Clear the temp message store 
                s.tempMessageStorage.clear();
                // Make a requestVote
                RequestMetadata requestMetadata = {
                    s.currentTerm,
                    s.nodeID,
                    s.commitIndex
                };
                // // Hash + Sign it
                // std::string msgDigestSigned = Crypto::SignData(requestMetadata.toString(), s.privateKey);
                // Append to response
                std::shared_ptr<RequestVote> requestMessage = std::make_shared<RequestVote>(requestMetadata, "msgDigestSigned");
                s.leaderProof = requestMessage;
                // Make RaftMessage
                std::shared_ptr<RaftMessage> raftMessage = std::make_shared<RaftMessage>(requestMessage);
                raftMessage -> dest = "*"; // broadcast
                raftMessage -> source = s.nodeID;
                // Push it to the ouput port
                s.raftOutMessages.emplace_back(raftMessage);
        }
    } 
    

    // Setter function to update nodeID inside RaftControllerModel..
    void setNodeID(const std::string& id) {
        state.nodeID = id;
    }

    // Setter function to update nodeID inside RaftControllerModel..
    void setPeers(const std::vector<std::string>& peers) {
            state.peers = peers;
    }


    // Setter function to update nodeID inside RaftControllerModel..
    RaftState&  getState() {
        return state;
    }



};


#endif