#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <cadmium/core/modeling/atomic.hpp>
#include <queue>
#include <string>
#include <iostream>
#include "../messages/network/network_message.hpp"
#include "../utils/stochastic/random.hpp"

using namespace cadmium;


struct NetworkState {
    std::priority_queue<std::shared_ptr<PacketEvent>, std::vector<std::shared_ptr<PacketEvent>>, ComparePacketEvent> packetQueue;  
    double currentTime = 0;

     friend std::ostream& operator<<(std::ostream& os, const NetworkState& s) {
        os << "NetworkState Queue Length: " <<  s.packetQueue.size() << "," <<
        "Current Time: " << s.currentTime;
        return os;
    }

};


// Network Atomic Model
class NetworkModel : public Atomic<NetworkState> {
public:

    Port<std::shared_ptr<Packet>> in_packet;  
    Port<std::shared_ptr<Packet>> out_packet; 

    // Constructor to initialize the Network model
    NetworkModel(const std::string& id) : Atomic<NetworkState>(id, {}) {
        in_packet = cadmium::Component::addInPort<std::shared_ptr<Packet>>("in_packet");
        out_packet = cadmium::Component::addOutPort<std::shared_ptr<Packet>>("out_packet");
    }

    void internalTransition(NetworkState& s) const override {
        if ( !s.packetQueue.empty() ) {
            s.packetQueue.pop();
        }
    }

    void externalTransition(NetworkState& s, double e) const override {
        s.currentTime += e;
        std::vector<std::shared_ptr<Packet>> packetListIn = in_packet -> getBag();
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
    void output(const NetworkState& s) const override {
        if (!s.packetQueue.empty()) {
            std::cout << "Forwarding packet: " << s.packetQueue.top() -> packet ;
            out_packet -> addMessage(s.packetQueue.top() -> packet); 
        }
    }

    double timeAdvance(const NetworkState& s) const override {
        // Return the smallest delay in the event queue
        return !s.packetQueue.empty() ? s.packetQueue.top() -> delay : std::numeric_limits<double>::infinity();
    }
};

#endif
