#ifndef RAFT_MESSAGES_HPP
#define RAFT_MESSAGES_HPP

#include "../messages.hpp"
#include "../util/heartbeat_messages.hpp"

enum class Task {VOTE_REQUEST, APPEND_ENTRIES, VOTE_RESPONSE};

class RaftMessage {
    public:
        explicit RaftMessage(std::shared_ptr<IMessage<Task>> _content) : content(std::move(_content)) {}
        std::shared_ptr<IMessage<Task>> content;
};




struct RequestMetadata {
    int termNumber;
    std::string candidateID;
    int lastLogIndex;

    std::string toString() const  {
        std::stringstream ss;
        ss << "RequestMetadata { "
           << "termNumber: " << std::to_string(termNumber) << ", "
           << "candidateID: \"" << candidateID << "\", "
           << "lastLogIndex: " << std::to_string(lastLogIndex) << " "
           << "}";
        return ss.str();
    }
};

class RequestVote : public IMessage<Task> {
    public:
        RequestVote() = default;
        RequestVote(RequestMetadata _metadata, std::string_view _msgDigestSigned) : metadata(_metadata), msgDigestSigned(_msgDigestSigned) {};

        RequestMetadata metadata;
        std::string msgDigestSigned;


        Task getType() override {
            return Task::VOTE_REQUEST;
        }

        std::string toString() const override {
            std::stringstream ss;
            ss << "RequestMetadata : {" 
            << metadata.toString() << "msgDigestSigned: " <<
            msgDigestSigned
            << "}";
            return ss.str();
        }
};

struct ResponseMetadata {
    int termNumber;
    std::string votedFor;
    int lastLogIndex;
    bool voteGranted;
    std::string nodeId;

    // Method to return a string representation of the struct
    std::string toString() const  
    {
        std::stringstream ss;
        ss << "ResponseMetadata { " << "termNumber: "  << termNumber << ", " << "votedFor: \"" << votedFor << "\", "
           << "lastLogIndex: " << lastLogIndex << ", " << "voteGranted: " << (voteGranted ? "true" : "false") << ", " 
           << "nodeId: \"" << nodeId << "\" "  << "}";
        return ss.str();
    }
};


class ResponseVote : public IMessage<Task>  {
    public:
        ResponseVote()  = default;
        ResponseVote(ResponseMetadata _metadata, std::string _msgDigestSigned) : metadata(_metadata), msgDigestSigned(_msgDigestSigned) {};
        
        ResponseMetadata metadata;
        std::string msgDigestSigned;

        Task getType() override {
            return Task::VOTE_RESPONSE;
        }
        std::string toString() const override {
            std::stringstream ss;
            ss << "ResponseVote : {" 
            << metadata.toString() << "msgDigestSigned: " <<
            msgDigestSigned
            << "}";
            return ss.str();
        }
};


enum class LogEntryType {RAFT, HEARTBEAT, EXTERNAL};


struct logEntryMetadata {
    RequestVote requestMessage;
    std::vector<ResponseVote> messageList;
};

class LogEntryRAFT : public  IMessage<LogEntryType>  
{
public:
    LogEntryRAFT() = default;
    LogEntryRAFT(logEntryMetadata _metadata) : metadata(_metadata) {};

    logEntryMetadata metadata;

    LogEntryType getType() override { return LogEntryType::RAFT; }

    std::string toString() const override 
    {
        std::stringstream ss;
        ss << "LogEntryRAFT : {" << "}";
        return ss.str();
    }
};

class LogEntryHeartbeat : public IMessage<LogEntryType> {
    public:
        LogEntryHeartbeat() = default;
        LogEntryHeartbeat(HeartbeatMetadata _metadata) : metadata(_metadata) {};

        HeartbeatMetadata metadata;  

        // IMessage<LogEntryType>& getContent() override { 
        //     return *this; 
        // }

        // Overriding getType to return the heartbeat log entry type
        LogEntryType getType() override {
            return LogEntryType::HEARTBEAT;  
        }

        // Convert the heartbeat log entry to a string suitable for the database
        std::string toString() const override {
            std::stringstream ss;
            ss << "LogEntryHeartbeat - Type: HEARTBEAT, Metadata: {" << metadata.toString() << "}";
            return ss.str();
        }
};


class LogEntryExternal : public IMessage<LogEntryType>  { 
    public:
        LogEntryExternal() = default;

        LogEntryType getType() override {
            return LogEntryType::EXTERNAL;
        }
};

struct AppendEntriesMetadata  {
    int term;          // Leader's current term
    std::string leaderID; // The ID of the leader
    int prevLogIndex;  // Index of log entry preceding the new entries
    int prevLogTerm;   // Term of the log entry at PrevLogIndex
    std::vector<std::shared_ptr<IMessage<LogEntryType>>> entries; // List of log entries to be replicated (empty for heartbeat)
    int leaderCommit;  // The index of the highest log entry known to be committed


    std::string toString() const {
        std::stringstream ss;
        ss << "AppendEntriesMetadata { " << "term: " << term << ", " << "leaderId: \"" << leaderID << "\", "
        << "prevLogIndex: " << prevLogIndex << ", "  << "prevLogTerm: " << prevLogTerm << ", " << "entries: [";
        
        for (size_t i = 0; i < entries.size(); ++i) {
            ss << entries[i]->toString();
            if (i < entries.size() - 1) ss << ", ";
        }

        ss << "], " << "leaderCommit: " << leaderCommit << " }";
        return ss.str();
    }

};

class AppendEntries : public IMessage<Task> {
    public:
        AppendEntries() = default;
        AppendEntries(AppendEntriesMetadata _metadata, std::string_view _msgDigestSigned) : metadata(_metadata), msgDigestSigned(_msgDigestSigned) {};
        AppendEntriesMetadata metadata;
        std::string msgDigestSigned;


        Task getType() override {
            return Task::APPEND_ENTRIES;
        }
        
        std::string toString() const override {
            std::stringstream ss;
            ss << "AppendEntries {" << metadata.toString() << "msgDigestSigned: " << msgDigestSigned << "}";
            return ss.str();
        }
};



#endif 