#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <cadmium/core/modeling/atomic.hpp>
#include <queue>
#include <unordered_map>
#include <string>
#include <iostream>
#include "../messages/network/network_message.hpp"
#include "../utils/stochastic/random.hpp"

using namespace cadmium;


struct NetworkState {
    std::priority_queue<std::shared_ptr<PacketEvent>, std::vector<std::shared_ptr<PacketEvent>>, ComparePacketEvent> packetQueue;  
    double currentTime = 0;
    std::vector<std::string> activeNodes;

     friend std::ostream& operator<<(std::ostream& os, const NetworkState& s) {
        os << "NetworkState Queue Length: " <<  s.packetQueue.size() << "," <<
        "Current Time: " << s.currentTime;
        return os;
    }

};


// Network Atomic Model
class NetworkModel : public Atomic<NetworkState> {
public:

    // <Destination address, port>
    std::unordered_map<std::string, Port<std::shared_ptr<Packet>>> input_ports;  
    std::unordered_map<std::string, Port<std::shared_ptr<Packet>>> output_ports; 


    // Constructor to initialize the Network model
    NetworkModel(const std::string& id, const std::vector<std::string> activeNodes) : Atomic<NetworkState>(id, {}) {
        state.activeNodes = activeNodes;
        for ( auto nodeID : activeNodes ) { 
            input_ports[nodeID] = cadmium::Component::addInPort<std::shared_ptr<Packet>>("input_packet_" + nodeID);
            output_ports[nodeID] = cadmium::Component::addOutPort<std::shared_ptr<Packet>>("output_packet_" + nodeID);
        }
    }

    void internalTransition(NetworkState& s) const override {
        if ( !s.packetQueue.empty() ) {
            s.packetQueue.pop();
        }
    }

    void externalTransition(NetworkState& s, double e) const override {
        s.currentTime += e;
        // Aggregate Bags
        for (auto node : s.activeNodes ) { 
            // Get the bag of incoming packets for this node
            auto bagAtPort = input_ports.at(node)->getBag(); 

            // Deal with external events
            for (auto packet : bagAtPort) {
                if (packet->destination == "*") {
                    for (auto node : s.activeNodes ) { 
                        if (node != packet -> source) {
                        std::shared_ptr<Packet>  packetNew = std::make_shared<Packet>(
                            packet -> payload,
                            node, 
                            packet -> source
                        );

                        std::shared_ptr<PacketEvent> packetEvent = std::make_shared<PacketEvent>(
                            packetNew,
                            RandomNumberGeneratorDEVS::generateExponentialDelay(1000000), 
                            s.currentTime
                        );

                        s.packetQueue.push(packetEvent);
                    }
                }
                } else {
                    std::shared_ptr<PacketEvent> packetEvent = std::make_shared<PacketEvent>(
                        packet,
                        RandomNumberGeneratorDEVS::generateExponentialDelay(1000000), 
                        s.currentTime
                    );
                    s.packetQueue.push(packetEvent);
                }
            }
        }
    }

    // Output: forward the current message after the delay
    void output(const NetworkState& s) const override {
        if (!s.packetQueue.empty()) {
            auto packet = s.packetQueue.top() -> packet;
            // Analyze the packet, check where it goes
            // Move it to appropriate port
            output_ports.at(packet -> destination) -> addMessage(s.packetQueue.top() -> packet); 

        }
    }

    double timeAdvance(const NetworkState& s) const override {
        // Return the smallest delay in the event queue
        return !s.packetQueue.empty() ? s.packetQueue.top() -> delay : std::numeric_limits<double>::infinity();
    }
};

#endif
