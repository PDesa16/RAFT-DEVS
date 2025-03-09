#ifndef SIMULATION_HPP
#define SIMULATION_HPP

#include <cadmium/core/modeling/coupled.hpp>
#include "node.hpp"
#include "../network.hpp"



using namespace cadmium;

class SimulationModel : public Coupled {
public:

    explicit SimulationModel(const std::string& id) : Coupled(id) {

        //  No external ports

        // Create instances of atomic models
        auto node0 = addComponent<NodeModel>("node0");
        auto node1 = addComponent<NodeModel>("node1");
        auto network = addComponent<NetworkModel>("network");

        auto test = node0 -> getOutPort("output_external");
        // auto test2 = network -> getInPort("in_packet") ;

        // Define couplings
        addCoupling(network -> getOutPort("out_packet"), node0 -> getInPort("external_input")); // Internal Coupling (IC)
        addCoupling(node0 -> getOutPort("output_external"), network -> getInPort("in_packet")); // Internal Coupling (IC)

    }
};

#endif