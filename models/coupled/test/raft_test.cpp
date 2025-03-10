#include <gtest/gtest.h>
#include "../raft.hpp"


class RaftFixture: public ::testing::Test
{
protected:
    std::unique_ptr<RaftModel> model;

    void SetUp() override 
    {
        InitModel();
    }

    void InitModel() 
    {
        model.reset();
        model = std::make_unique<RaftModel>("buffer-raft");
    }

};


// Test Model
TEST_F(RaftFixture, testBuildBufferRaftCoupledModel) {
    std::cout << "test";
    ASSERT_TRUE(model != nullptr);
}


// Main function for Google Test
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv); 
    return RUN_ALL_TESTS();
}