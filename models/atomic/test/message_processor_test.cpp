#include <gtest/gtest.h>
#include "../message_processor.hpp"


class MessageProcessorAtomicFixture: public ::testing::Test
{
protected:
    std::unique_ptr<MessageProcessorModel> model;
    MessageProcessorState state{}; 

    void SetUp() override 
    {
        InitModel();
    }

    void InitModel() 
    {
        model.reset();
        model = std::make_unique<MessageProcessorModel>("message-processor");
    }

};


// Test Model
TEST_F(MessageProcessorAtomicFixture, testBuildNetworkAtomicModel) {
    ASSERT_TRUE(model != nullptr);
}


// Main function for Google Test
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv); 
    return RUN_ALL_TESTS();
}