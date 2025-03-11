#ifndef SIMULATION_HPP
#define SIMULATION_HPP

#include <cadmium/core/modeling/coupled.hpp>
#include "node.hpp"
#include "../atomic/network.hpp"
#include <unordered_map>



using namespace cadmium;


class SimulationModel : public Coupled {
public:

    explicit SimulationModel(const std::string& id) : Coupled(id) {


        std::vector<std::string> nodesID;
        std::unordered_map<std::string, std::shared_ptr<NodeModel>> nodes;
        

        for (int i = 0 ; i < 3; i++){
            auto nodeID = "node" + std::to_string(i);
            nodesID.push_back(nodeID); 
            nodes[nodeID] = addComponent<NodeModel>(nodeID);
        }


        auto network = addComponent<NetworkModel>("network", nodesID);

        for (auto nodeID : nodesID) {
            auto raftChild = nodes[nodeID] -> getComponent("raft");
            auto raftChildController = std::dynamic_pointer_cast<RaftModel>(raftChild) -> getComponent("raft-controller");
            std::vector<std::string> peers;
            // Copy all elements except `nodeID`
            std::copy_if(nodesID.begin(), nodesID.end(), std::back_inserter(peers), 
                         [nodeID](const std::string& x) { return x != nodeID; });
            
            // Pass `peers` to setPeers()
            std::dynamic_pointer_cast<RaftControllerModel>(raftChildController)->setPeers(peers);
            

            addCoupling(network -> getOutPort("output_packet_"+ nodeID), nodes[nodeID] -> getInPort("external_input")); // Internal Coupling (IC)
            addCoupling(nodes[nodeID] -> getOutPort("output_external"), network -> getInPort("input_packet_" + nodeID)); // Internal Coupling (IC)
        }

    };

};

#endif