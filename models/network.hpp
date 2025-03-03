#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <cadmium/core/modeling/atomic.hpp>
#include <queue>
#include <string>
#include <iostream>

using namespace cadmium;

// NetworkState will hold the current message in the network
struct NetworkState {
    std::string currentMessage;  // Store the current message
    double delay;  // Store the delay time
};

// Network Atomic Model
class Network : public Atomic<NetworkState> {
public:
    // Declare the input and output ports for the network
    Port<std::string> in_port;  // Input port for incoming messages
    Port<std::string> out_port; // Output port for outgoing messages

    // Constructor to initialize the Network model
    Network(const std::string& id) : Atomic<NetworkState>(id, {}) {}

    // Internal transition: we perform no action when no new input arrives
    void internalTransition(NetworkState& s) const override {
        // Reset the current message and delay after the internal transition
        s.currentMessage.clear();
        s.delay = 0;
    }

    // External transition: add delay and store the incoming message
    void externalTransition(NetworkState& s, double e) const override {

        std::vector<std::string> packetList = in_port -> getBag();
        for (auto& packet : packetList) {
            // Add delay
            s.currentMessage = packet;
            s.delay = 2.0;  
        }
    }

    // Output: forward the current message after the delay
    void output(const NetworkState& s) const override {
        if (!s.currentMessage.empty()) {
            // Print the message to simulate forwarding it to the proper node
            std::cout << "Forwarding message: " << s.currentMessage;
            out_port -> addMessage(s.currentMessage);  // Send the message to the output port
        }
    }

    // Time advance: return the delay time until the next event
    double timeAdvance(const NetworkState& s) const override {
        // If there's a delay, return the delay time; otherwise, no delay (0)
        return (s.delay > 0) ? s.delay : 0;
    }
};

#endif
