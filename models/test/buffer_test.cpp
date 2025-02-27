#include <gtest/gtest.h>
#include "../buffer.hpp"


TEST(TestBuffer, TestBufferInit) {
    //Defining buffer state
    const BufferState<std::string> bufferState;
    // Creating buffer model
    Buffer<std::string>* bufferModel = new Buffer<std::string>("nodeBuffer");
    bufferModel -> timeAdvance(bufferState);
    ASSERT_TRUE(1);
}

// Main function for Google Test
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv); 
    return RUN_ALL_TESTS();
}