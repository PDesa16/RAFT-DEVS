#include <gtest/gtest.h>
#include "../node.hpp"


class NodeCoupledFixture: public ::testing::Test
{
protected:
    std::unique_ptr<NodeModel> model;

    void SetUp() override 
    {
        InitModel();
    }

    void InitModel() 
    {
        model.reset();
        model = std::make_unique<NodeModel>("node");
    }

};


// Test Model
TEST_F(NodeCoupledFixture, testBuildBufferRaftCoupledModel) {
    std::cout << "test";
    ASSERT_TRUE(model != nullptr);
}


// Main function for Google Test
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv); 
    return RUN_ALL_TESTS();
}