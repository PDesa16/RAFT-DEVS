#ifndef HEARTBEAT_MESSAGES_HPP
#define HEARTBEAT_MESSAGES_HPP

// Create the heartbeat content
class HeartbeatMessage {
    HeartBeatMetadata metadata;
    std::string ack; // message digest signed
}

enum class HEARTBEAT_STATUS {PING, ECHO_RESPONSE} 
class HeartBeatMetadata {
    int senderId;
    int sequenceNumber;
    double timestamp;
    HEARTBEAT_STATUS status;
}


#endif