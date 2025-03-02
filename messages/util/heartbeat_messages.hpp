#ifndef HEARTBEAT_MESSAGES_HPP
#define HEARTBEAT_MESSAGES_HPP

#include <string>
#include <sstream>


enum class HEARTBEAT_STATUS {PING, ECHO_RESPONSE};

// Convert HEARTBEAT_STATUS to string
std::string heartbeatStatusToString(HEARTBEAT_STATUS status) {
    switch (status) {
        case HEARTBEAT_STATUS::PING: return "PING";
        case HEARTBEAT_STATUS::ECHO_RESPONSE: return "ECHO_RESPONSE";
        default: return "UNKNOWN";
    }
};

// HeartBeatMetadata class that contains information about the heartbeat
class HeartbeatMetadata {
public:
    std::string senderId;
    int sequenceNumber;
    double timestamp;
    HEARTBEAT_STATUS status;

    // Convert the metadata to a readable string format
    std::string toString() const {
        std::stringstream ss;
        ss << "Sender ID: " << senderId << ", "
           << "Sequence Number: " << sequenceNumber << ", "
           << "Timestamp: " << timestamp << ", "
           << "Status: " << heartbeatStatusToString(status);
        return ss.str();
    }
};

// Create the heartbeat content
class HeartbeatMessage {
    HeartbeatMetadata metadata;
    std::string ack; // message digest signed
};

#endif