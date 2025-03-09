#ifndef PACKET_PROCESSOR_HPP
#define PACKET_PROCESSOR_HPP

#include <cadmium/core/modeling/atomic.hpp>
#include <queue>
#include <string>
#include <iostream>
#include "../messages/network/network_message.hpp"
#include "../utils/stochastic/random.hpp"
#include "../messages/raft/raft_messages.hpp"

using namespace cadmium;


struct PacketProcessorState {
    std::priority_queue<std::shared_ptr<PacketEvent>, std::vector<std::shared_ptr<PacketEvent>>, ComparePacketEvent> packetQueue;  
    double currentTime = 0;

     friend std::ostream& operator<<(std::ostream& os, const PacketProcessorState& s) {
        os << "NetworkState Queue Length: " <<  s.packetQueue.size() << "," <<
        "Current Time: " << s.currentTime;
        return os;
    }

};


// Network Atomic Model
class PacketProcessorModel : public Atomic<PacketProcessorState> {
public:

    Port<std::shared_ptr<Packet>> input_packet;
    Port<std::shared_ptr<RaftMessage>> output_raft_message; 

    // Constructor to initialize the Network model
    PacketProcessorModel(const std::string& id) : Atomic<PacketProcessorState>(id, {}){
        input_packet = cadmium::Component::addInPort<std::shared_ptr<Packet>>("input_packet");
        output_raft_message = cadmium::Component::addOutPort<std::shared_ptr<RaftMessage>>("output_raft_message");
    }

    void internalTransition(PacketProcessorState& s) const override {
        if ( !s.packetQueue.empty() ) {
            s.packetQueue.pop();
        }
    }

    void externalTransition(PacketProcessorState& s, double e) const override {
        s.currentTime += e;
        std::vector<std::shared_ptr<Packet>> packetListIn = input_packet -> getBag();
        for (auto packet : packetListIn) {
            std::shared_ptr<PacketEvent> packetEvent = std::make_shared<PacketEvent>(
                packet,
                RandomNumberGeneratorDEVS::generateExponentialDelay(1000000), 
                s.currentTime
            );
            s.packetQueue.push(packetEvent);
        }
    }


    // Output: forward the current message after the delay
    void output(const PacketProcessorState& s) const override {
        if (!s.packetQueue.empty()) {
            std::cout << "Forwarding message: " << s.packetQueue.top() -> packet ;
            auto message = s.packetQueue.top() -> packet -> payload;
            switch (message -> getType()) {
                case PacketPayloadType::RAFT:
                    output_raft_message -> addMessage(std::static_pointer_cast<RaftMessage>(message)); 
                    break;
                default:
                    break;
                }
        }
    }

    double timeAdvance(const PacketProcessorState& s) const override {
        return !s.packetQueue.empty() ? s.packetQueue.top() -> delay : std::numeric_limits<double>::infinity();
    }
};

#endif
