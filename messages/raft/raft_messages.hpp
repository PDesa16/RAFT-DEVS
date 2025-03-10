#ifndef RAFT_MESSAGES_HPP
#define RAFT_MESSAGES_HPP

#include "../messages.hpp"
#include "../util/heartbeat_messages.hpp"

enum class HeartbeatStatus {ALIVE, TIMEOUT, UPDATE, INIT};

std::ostream& operator<<(std::ostream& os, HeartbeatStatus status) {
    os << (status == HeartbeatStatus::ALIVE ? "ALIVE" : "TIMEOUT");
    return os;
}



enum class Task {VOTE_REQUEST, APPEND_ENTRIES, VOTE_RESPONSE};

class RaftMessage : public IMessage<PacketPayloadType> {
    public:
        RaftMessage(std::shared_ptr<IMessage<Task>> _content) : content(std::move(_content)) {}
        RaftMessage() : content(nullptr) {}
        std::shared_ptr<IMessage<Task>> content;
        std::string source = "";
        std::string dest = "";

        PacketPayloadType getType() override {
            return PacketPayloadType::RAFT;
        };

        std::string toString() const override {
            std::stringstream ss;
            ss << "RaftMessage { "
               << "source: \"" << source << "\", "
               << "dest: \"" << dest << "\", "
               << "content: " << (content ? content->toString() : "null")
               << " }";
            return ss.str();
        };

        friend std::ostream& operator<<(std::ostream& os, const RaftMessage& msg) {
            os << msg.toString();
            return os;
        }
};

struct RequestMetadata {
    int termNumber;
    std::string candidateID;
    int lastLogIndex;

    std::string toString() const {
        std::stringstream ss;
        ss << "RequestMetadata { "
           << "termNumber: " << termNumber << ", "
           << "candidateID: \"" << candidateID << "\", "
           << "lastLogIndex: " << lastLogIndex
           << " }";
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
            ss << "RequestVote { "
               << "metadata: {" << metadata.toString() << "}, "
               << "msgDigestSigned: \"" << msgDigestSigned << "\""
               << " }";
            return ss.str();
        }
};

struct ResponseMetadata {
    int termNumber;
    std::string votedFor;
    int lastLogIndex;
    bool voteGranted;
    std::string nodeId;

    std::string toString() const {
        std::stringstream ss;
        ss << "ResponseMetadata { "
           << "termNumber: " << termNumber << ", "
           << "votedFor: \"" << votedFor << "\", "
           << "lastLogIndex: " << lastLogIndex << ", "
           << "voteGranted: " << (voteGranted ? "true" : "false") << ", "
           << "nodeId: \"" << nodeId << "\""
           << " }";
        return ss.str();
    }
};

class ResponseVote : public IMessage<Task> {
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
            ss << "ResponseVote { "
               << "metadata: {" << metadata.toString() << "}, "
               << "msgDigestSigned: \"" << msgDigestSigned << "\""
               << " }";
            return ss.str();
        }
};

enum class LogEntryType {RAFT, HEARTBEAT, EXTERNAL};

struct logEntryMetadata {
    RequestVote requestMessage;
    std::vector<ResponseVote> messageList;
};

class LogEntryRAFT : public IMessage<LogEntryType> {
public:
    LogEntryRAFT() = default;
    LogEntryRAFT(logEntryMetadata _metadata) : metadata(_metadata) {};

    logEntryMetadata metadata;

    LogEntryType getType() override { return LogEntryType::RAFT; }

    std::string toString() const override {
        std::stringstream ss;
        ss << "LogEntryRAFT { "
           << "requestMessage: {" << metadata.requestMessage.toString() << "}, "
           << "messageList: [";
        for (const auto& msg : metadata.messageList) {
            ss << msg.toString() << ", ";
        }
        ss << "] }";
        return ss.str();
    }
};

class LogEntryHeartbeat : public IMessage<LogEntryType> {
    public:
        LogEntryHeartbeat() = default;
        LogEntryHeartbeat(HeartbeatMetadata _metadata) : metadata(_metadata) {};

        HeartbeatMetadata metadata;

        LogEntryType getType() override {
            return LogEntryType::HEARTBEAT;
        }

        std::string toString() const override {
            std::stringstream ss;
            ss << "LogEntryHeartbeat { "
               << "metadata: {" << metadata.toString() << "}"
               << " }";
            return ss.str();
        }
};

class LogEntryExternal : public IMessage<LogEntryType>  { 
    public:
        LogEntryExternal() = default;

        LogEntryType getType() override {
            return LogEntryType::EXTERNAL;
        }

        std::string toString() const override {
            return "LogEntryExternal { }";
        }
};

struct AppendEntriesMetadata {
    int term;          // Leader's current term
    std::string leaderID; // The ID of the leader
    int prevLogIndex;  // Index of log entry preceding the new entries
    int prevLogTerm;   // Term of the log entry at PrevLogIndex
    std::vector<std::shared_ptr<IMessage<LogEntryType>>> entries; // List of log entries to be replicated (empty for heartbeat)
    int leaderCommit;  // The index of the highest log entry known to be committed

    std::string toString() const {
        std::stringstream ss;
        ss << "AppendEntriesMetadata { "
           << "term: " << term << ", "
           << "leaderId: \"" << leaderID << "\", "
           << "prevLogIndex: " << prevLogIndex << ", "
           << "prevLogTerm: " << prevLogTerm << ", "
           << "entries: [";
        
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
            ss << "AppendEntries { "
               << "metadata: {" << metadata.toString() << "}, "
               << "msgDigestSigned: \"" << msgDigestSigned << "\""
               << " }";
            return ss.str();
        }
};

#endif
