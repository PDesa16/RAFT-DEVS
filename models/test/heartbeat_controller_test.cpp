#include <gtest/gtest.h>
#include "../heartbeat_controller.hpp"
#include <limits>

// Test fixture for the HeartbeatControllerModel
class HeartbeatControllerAtomicFixture : public ::testing::Test {
protected:
    std::unique_ptr<HeartbeatControllerModel> model;
    HeartbeatControllerState state{}; 

    void SetUp() override {
        InitModel();
    }

    void InitModel() {
        model.reset();
        model = std::make_unique<HeartbeatControllerModel>("heartbeatController");
    }
};

// Test initialization of the model
TEST_F(HeartbeatControllerAtomicFixture, testBuildNetworkAtomicModel) {
    ASSERT_TRUE(model != nullptr);
    ASSERT_NE(state.heartbeatTimeout, std::numeric_limits<double>::infinity());  // Initially, the timeout should be random ~ 150-300ms
}

// Test external transition for ALIVE heartbeat update
TEST_F(HeartbeatControllerAtomicFixture, testExternalTransitionAlive) {
    // Simulate an ALIVE heartbeat update
    model->input_heartbeat_update->addMessage(HeartbeatStatus::ALIVE);
    model->externalTransition(state, 0.350); // Assume 350ms for time
    
    // Check that the heartbeatTimeout was set to a random value between 0.150 and 0.300
    ASSERT_GT(state.heartbeatTimeout, 0.150);
    ASSERT_LT(state.heartbeatTimeout, 0.300);
}

// Test internal transition
TEST_F(HeartbeatControllerAtomicFixture, testInternalTransition) {
    // Simulate an internal transition
    model->internalTransition(state);
    
    // After the internal transition, the heartbeatTimeout should be infinity
    ASSERT_EQ(state.heartbeatTimeout, std::numeric_limits<double>::infinity());
}

// Test output when timeout is reached
TEST_F(HeartbeatControllerAtomicFixture, testOutputTimeout) {
    // Set the heartbeat timeout to 0 to trigger output
    state.heartbeatTimeout = 0;

    // Call the output function
    model->output(state);

    // Check that the output message was added
    auto messages = model->output_heartbeat_timeout->getBag();
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], HeartbeatStatus::TIMEOUT);
}

// Test time advance
TEST_F(HeartbeatControllerAtomicFixture, testTimeAdvance) {
    // Set a specific heartbeat timeout
    state.heartbeatTimeout = 0.25;

    // Check that the time advance returns the correct timeout value
    ASSERT_EQ(model->timeAdvance(state), 0.25);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv); 
    return RUN_ALL_TESTS();
}
