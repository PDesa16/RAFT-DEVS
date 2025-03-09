#include <gtest/gtest.h>
#include "../buffer_raft.hpp"


class BufferRaftAtomicFixture: public ::testing::Test
{
protected:
    std::unique_ptr<BufferRaftModel> model;

    void SetUp() override 
    {
        InitModel();
    }

    void InitModel() 
    {
        model.reset();
        model = std::make_unique<BufferRaftModel>("buffer-raft");
    }

};


// Test Model
TEST_F(BufferRaftAtomicFixture, testBuildBufferRaftCoupledModel) {
    std::cout << "test";
    ASSERT_TRUE(model != nullptr);
}


// Main function for Google Test
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv); 
    return RUN_ALL_TESTS();
}