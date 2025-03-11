#ifndef NODE_RAFT_HPP
#define NODE_RAFT_HPP

#include <cadmium/core/modeling/coupled.hpp>
#include "raft.hpp" 
#include "../atomic/packet_processor.hpp"
#include "../atomic/message_processor.hpp"


using namespace cadmium;

class NodeModel : public Coupled {
public:

    // Port<Packet> in_packet;  
    // Port<Packet> out_packet; 

    explicit NodeModel(const std::string& id) : Coupled(id) {


        addInPort<std::shared_ptr<Packet>>("external_input");
		addOutPort<std::shared_ptr<Packet>>("output_external");

        // Create instances of atomic models
        auto raft = addComponent<RaftModel>("raft");
        auto messageProcessor = addComponent<MessageProcessorModel>("message-processor");
        auto packetProcessor = addComponent<PacketProcessorModel>("packet-processor");

        // Pass the node id information to RAFT
        auto raftController = raft -> getComponent("raft-controller");
        std::dynamic_pointer_cast<RaftControllerModel>(raftController)->setNodeID(id);



        // Define couplings
        addCoupling(raft -> getOutPort("output_external"), messageProcessor -> getInPort("input_raft_message")); // Internal Coupling (IC)
        addCoupling(packetProcessor -> getOutPort("output_raft_message"), raft -> getInPort("external_input")); // Internal Coupling (IC)
        addEIC(getInPort("external_input"), packetProcessor -> getInPort("input_packet"));     // External Input Coupling (EIC)
        addEOC(messageProcessor ->getOutPort("output_packet"), getOutPort("output_external")); // External Output Coupling (EOC)

    }
};

#endif