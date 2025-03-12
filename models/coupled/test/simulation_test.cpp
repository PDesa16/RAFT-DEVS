#include <gtest/gtest.h>
#include "../simulation.hpp"
#include "../../../logger/raft_logger.hpp"
#include <cadmium/core/simulation/root_coordinator.hpp>


class SimulationFixture: public ::testing::Test
{
protected:
    std::unique_ptr<SimulationModel> model;

    void SetUp() override 
    {
        InitModel();
    }

    void InitModel() 
    {
        model.reset();
        model = std::make_unique<SimulationModel>("simulation");
    }


};


// Test Model
TEST_F(SimulationFixture, testBuildBufferRaftCoupledModel) {
    // Create a RootCoordinator to manage the simulation
    // Instantiate the top-level coupled model
    auto logger = std::make_shared<RAFTLogger>();
    auto model = std::make_shared<SimulationModel>("simulation");
    RootCoordinator root(model);
    root.setLogger(logger);
    // Set the simulation time limit (e.g., 1000 time units)
    root.simulate(0.3);
    ASSERT_TRUE(model != nullptr);

}


// Main function for Google Test
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv); 
    return RUN_ALL_TESTS();
}