#ifndef RAFT_MESSAGES_HPP
#define RAFT_MESSAGES_HPP

#include "../messages.hpp"
#include "../util/heartbeat_messages.hpp"

enum class Task {VOTE_REQUEST, APPEND_ENTRIES, VOTE_RESPONSE};

struct MessageRaft {
    BaseMessageContentInterface<Task> content;
}

// Heartbeat Message
class HeartbeatMessageRaft : BaseMessageContentInterface<Task> {
    HeartbeatMessage heartbeatMessage;

    BaseMessageContentInterface<Task>& getContent() override { 
        return this;
    }

    Task getType() override {
        return Task::HEARTBEAT;
    }
}

//
class RequestVote : BaseMessageContentInterface<Task> {
    RequestMetadata metadata;
    std::string msgDigestSigned

    BaseMessageContentInterface<Task>& getContent() override { 
        return this;
    }

    Task getType() override {
        return Task::VOTE_REQUEST;
    }
}

class RequestMetadata {
    int termNumber;
    int candidateID;
    int lastLogIndex;
}

// 
class ResponseVote : BaseMessageContentInterface<Task> {
    ResponseMetadata metadata;
    std::string msgDigestSigned;

    BaseMessageContentInterface<Task>& getContent() override { 
        return this;
    }

    Task getType() override {
        return Task::VOTE_RESPONSE;
    }
}

class ResponseMetadata {
    int termNumber;
    int votedFor;
    int lastLogIndex;
    bool voteGrated;
    int nodeId;
}


enum class LogEntryType {RAFT, HEARTBEAT, EXTERNAL}
struct LogEntryRAFT : BaseMessageContentInterface<LogEntryType>  {
    BaseMessageContentInterface<Task>& getContent() override { 
        return this;
    }

    Task getType() override {
        return LogEntryType::RAFT;
    }
};

struct LogEntryHeartbeat : BaseMessageContentInterface<LogEntryType>  {
    BaseMessageContentInterface<Task>& getContent() override { 
        return this;
    }

    Task getType() override {
        return LogEntryType::HEARTBEAT;
    }
};

struct LogEntryExternal : BaseMessageContentInterface<LogEntryType>  {
    BaseMessageContentInterface<Task>& getContent() override { 
        return this;
    }

    Task getType() override {
        return LogEntryType::EXTERNAL;
    }
};

struct AppendEntriesMetadata  {
    int term;          // Leader's current term
    std::string leaderId; // The ID of the leader
    int prevLogIndex;  // Index of log entry preceding the new entries
    int prevLogTerm;   // Term of the log entry at PrevLogIndex
    std::vector<BaseMessageContentInterface<LogEntryType>> entries; // List of log entries to be replicated (empty for heartbeat)
    int leaderCommit;  // The index of the highest log entry known to be committed
}

struct AppendEntries : BaseMessageContentInterface<Task> {
    AppendEntriesMetadata metadata;
    std::string msgDigestSigned;

    BaseMessageContentInterface<Task>& getContent() override { 
        return this;
    }

    Task getType() override {
        return Task::APPEND_ENTRIES;
    }
};



#endif 