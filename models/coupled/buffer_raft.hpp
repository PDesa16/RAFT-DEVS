#ifndef BUFFER_RAFT_HPP
#define BUFFER_RAFT_HPP

#include <cadmium/core/modeling/coupled.hpp>
#include "../buffer.hpp" 
#include "../raft.hpp"

using namespace cadmium;

class BufferRaftModel : public Coupled {
public:

    // Port<RaftMessage> external_input;  
    // Port<RaftMessage> output_external; 

    explicit BufferRaftModel(const std::string& id) : Coupled(id) {


        addInPort<std::shared_ptr<RaftMessage>>("external_input");
		addOutPort<std::shared_ptr<RaftMessage>>("output_external");


        // addInPort("external_input");   // Input port for the coupled model
        // addOutPort("out_external");    // Output port for the coupled model

        // Create instances of atomic models
        auto buffer = addComponent<Buffer<RaftMessage>>("buffer");
        auto raft = addComponent<RaftModel>("raft");

        // addCoupling(getInPort("in"), buffer->);

        // Define couplings
        addCoupling(buffer -> getOutPort("output_buffer"), raft -> getInPort("input_buffer")); // Internal Coupling (IC)
        addEIC(getInPort("external_input"), buffer -> getInPort("input_buffer"));     // External Input Coupling (EIC)
        addEOC(raft ->getOutPort("output_external"), getOutPort("output_external")); // External Output Coupling (EOC)

    }
};

#endif