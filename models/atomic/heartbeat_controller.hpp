#ifndef HEARTBEAT_CONTROLLER_HPP
#define HEARTBEAT_CONTROLLER_HPP

#include <cadmium/core/modeling/atomic.hpp>
#include <iostream>
#include <vector>
#include <memory>
#include "../../utils/stochastic/random.hpp"
#include "../../messages/raft/raft_messages.hpp"

using namespace cadmium;



// HeartbeatController State
struct HeartbeatControllerState {
    HeartbeatStatus status = HeartbeatStatus::ALIVE;
    double heartbeatTimeout = RandomNumberGeneratorDEVS::generateUniformDelay(0.150,0.300);   // Time until next timeout

    friend std::ostream& operator<<(std::ostream& os, const HeartbeatControllerState& s) {
        os << "HeartbeatControllerState Timeout: " <<  s.heartbeatTimeout;
        return os;
    }
};

// HeartbeatController Atomic Model
class HeartbeatControllerModel : public Atomic<HeartbeatControllerState> {
public:
    // Ports
    Port<HeartbeatStatus> input_heartbeat_update;  
    Port<HeartbeatStatus> output_heartbeat_timeout; 

    // Constructor to initialize the HeartbeatController model
    HeartbeatControllerModel(const std::string& id) : Atomic<HeartbeatControllerState>(id, {}) {
        input_heartbeat_update = cadmium::Component::addInPort<HeartbeatStatus>("input_heartbeat");
        output_heartbeat_timeout = cadmium::Component::addOutPort<HeartbeatStatus>("output_heartbeat");
    }

    // Internal transition: Reset the heartbeat timeout when a timeout event occurs
    void internalTransition(HeartbeatControllerState& s) const override {
        // Heartbeat sent out, now waiting for the next external heartbeat update
        // Keep looping every 50ms otherwise..
        if (s.status !=  HeartbeatStatus::UPDATE) {
            s.heartbeatTimeout = std::numeric_limits<double>::infinity();
        }

    }

    // External transition: Start countdown for heartbeat timeout
    void externalTransition(HeartbeatControllerState& s, double e) const override { 
        s.status = input_heartbeat_update->getBag()[0]; // Assuming the heartbeat update is ALIVE or TIMEOUT
        if (s.status == HeartbeatStatus::ALIVE) {
            // Set heartbeat timeout 
            s.heartbeatTimeout = RandomNumberGeneratorDEVS::generateUniformDelay(0.150,0.300);  // Example: Set timeout duration to 5 seconds
        } else if (s.status == HeartbeatStatus::UPDATE) {
            // Update in 50ms
            s.heartbeatTimeout = 0.05;
        }
    }

    // Output: Send timeout event when the timeout is reached
    void output(const HeartbeatControllerState& s) const override {
        // Send the timeout message when the timeout expires
        if (s.status == HeartbeatStatus::ALIVE) { 
            output_heartbeat_timeout->addMessage(HeartbeatStatus::TIMEOUT);
        } else if (s.status == HeartbeatStatus::UPDATE) {
            output_heartbeat_timeout->addMessage(HeartbeatStatus::UPDATE);
        }
        
    }

    // Time advance: Wait for the remaining time before next event
    double timeAdvance(const HeartbeatControllerState& s) const override {
        return s.heartbeatTimeout;  // Return the remaining time until the next timeout
    }
};

#endif
