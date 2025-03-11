#ifndef RAFT_HPP
#define RAFT_HPP

#include <cadmium/core/modeling/coupled.hpp>
#include "../atomic/buffer.hpp" 
#include "../atomic/raft_controller.hpp"
#include "../atomic/heartbeat_controller.hpp"



using namespace cadmium;

class RaftModel : public Coupled {
public:

    // Port<Packet> in_packet;  
    // Port<Packet> out_packet; 

    explicit RaftModel(const std::string& id) : Coupled(id) {


        addInPort<std::shared_ptr<RaftMessage>>("external_input");
		addOutPort<std::shared_ptr<RaftMessage>>("output_external");

        // Create instances of atomic models
        auto raftController = addComponent<RaftControllerModel>("raft-controller");
        auto heartbeatController = addComponent<HeartbeatControllerModel>("heartbeat-controller");
        auto buffer = addComponent<Buffer<RaftMessage>>("buffer");

        // Define couplings
        addCoupling(buffer -> getOutPort("output_buffer"), raftController -> getInPort("input_buffer")); // Internal Coupling (IC)
        addCoupling(raftController -> getOutPort("output_heartbeat"), heartbeatController -> getInPort("input_heartbeat")); // Internal Coupling (IC)
        addCoupling(heartbeatController -> getOutPort("output_heartbeat"), raftController -> getInPort("input_heartbeat")); // Internal Coupling (IC)
        addEIC(getInPort("external_input"), buffer -> getInPort("input_buffer"));     // External Input Coupling (EIC)
        addEOC(raftController ->getOutPort("output_external"), getOutPort("output_external")); // External Output Coupling (EOC)

    }
};

#endif
