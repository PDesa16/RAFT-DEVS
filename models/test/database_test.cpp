#include <gtest/gtest.h>
#include "../database.hpp"


TEST(TestDatabase, TestDatabaseInit) {
    //Defining buffer state
    const DatabaseState<std::string, std::string> databaseState;
    // Creating buffer model
    Database<std::string, std::string>* databaseModel = new Database<std::string, std::string>("databaseModel");
    databaseModel -> timeAdvance(databaseState);
    ASSERT_TRUE(1);
}

// Main function for Google Test
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv); 
    return RUN_ALL_TESTS();
}