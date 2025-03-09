#ifndef MESSAGE_PROCESSOR_HPP
#define MESSAGE_PROCESSOR_HPP

#include <cadmium/core/modeling/atomic.hpp>
#include <queue>
#include <string>
#include <iostream>
#include "../messages/network/network_message.hpp"
#include "../utils/stochastic/random.hpp"
#include "../messages/raft/raft_messages.hpp"

using namespace cadmium;



struct MessageEvent {
    public:
        MessageEvent(std::shared_ptr<RaftMessage> _raft, double _delay, double _dispatchTime) : message(std::move(_raft)), delay(_delay),
        dispatchTime(_dispatchTime) {}
        std::shared_ptr<RaftMessage> message;
        double delay;
        double dispatchTime;

        // Min check
        bool operator<(const MessageEvent& b) const {
            return (this->dispatchTime + this->delay) > (b.dispatchTime + b.delay); 
        }

};

struct CompareMessageEvent {
    bool operator()(const std::shared_ptr<MessageEvent>& a, const std::shared_ptr<MessageEvent>& b) const {
        return *a < *b;  // Dereference shared_ptr to compare PacketEvent objects
    }
};

struct MessageProcessorState {
    std::priority_queue<std::shared_ptr<MessageEvent>, std::vector<std::shared_ptr<MessageEvent>>, CompareMessageEvent> messageQueue;  
    double currentTime = 0;

     friend std::ostream& operator<<(std::ostream& os, const MessageProcessorState& s) {
        os << "MessageState Queue Length: " <<  s.messageQueue.size() << "," <<
        "Current Time: " << s.currentTime;
        return os;
    }

};


// Network Atomic Model
class MessageProcessorModel : public Atomic<MessageProcessorState> {
public:

    Port<std::shared_ptr<RaftMessage>> in_raft_message; 
    Port<std::shared_ptr<Packet>> out_packet;
    
    // Constructor to initialize the Network model
    MessageProcessorModel(const std::string& id) : Atomic<MessageProcessorState>(id, {}){
        in_raft_message = cadmium::Component::addInPort<std::shared_ptr<RaftMessage>>("input_raft_message");
        out_packet = cadmium::Component::addOutPort<std::shared_ptr<Packet>>("output_packet");
    }
    

    void internalTransition(MessageProcessorState& s) const override {
        if ( !s.messageQueue.empty() ) {
            s.messageQueue.pop();
        }
    }

    void externalTransition(MessageProcessorState& s, double e) const override {
        s.currentTime += e;
        std::vector<std::shared_ptr<RaftMessage>> raftMessagesIn = in_raft_message -> getBag();
        for (auto raftMessage : raftMessagesIn) {
            std::shared_ptr<MessageEvent> packetEvent = std::make_shared<MessageEvent>(
                raftMessage,
                RandomNumberGeneratorDEVS::generateExponentialDelay(1000000), 
                s.currentTime
            );
            s.messageQueue.push(packetEvent);
        }
    }


    // Output: forward the current message after the delay
    void output(const MessageProcessorState& s) const override {
        if (!s.messageQueue.empty()) {
            std::shared_ptr<Packet> packet = std::make_shared<Packet>(
                s.messageQueue.top()-> message,
                s.messageQueue.top()->message->dest,
                s.messageQueue.top()->message->source
            );

            out_packet -> addMessage(packet); 
        }
    }

    double timeAdvance(const MessageProcessorState& s) const override {
        return !s.messageQueue.empty() ? s.messageQueue.top() -> delay : std::numeric_limits<double>::infinity();
    }
};

#endif
