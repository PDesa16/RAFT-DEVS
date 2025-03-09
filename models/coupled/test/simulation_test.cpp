#include <gtest/gtest.h>
#include "../simulation.hpp"


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
    std::cout << "test";
    ASSERT_TRUE(model != nullptr);
}


// Main function for Google Test
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv); 
    return RUN_ALL_TESTS();
}