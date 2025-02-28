#ifndef RAFT_MESSAGES_HPP
#define RAFT_MESSAGES_HPP

#include "../messages.hpp"
#include "../util/heartbeat_messages.hpp"

enum class Task { VOTE_REQUEST, APPEND_ENTRIES, VOTE_RESPONSE, HEARTBEAT};

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
    int source;

    BaseMessageContentInterface<Task>& getContent() override { 
        return this;
    }

    Task getType() override {
        return Task::VOTE_REQUEST;
    }
}

// 
class ResponseVote : BaseMessageContentInterface<Task> {
    int source;
    int destination;

    BaseMessageContentInterface<Task>& getContent() override { 
        return this;
    }

    Task getType() override {
        return Task::VOTE_RESPONSE;
    }
}

/* */



#endif 