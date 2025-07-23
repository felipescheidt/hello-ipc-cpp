#include "LedManager.hpp"
#include "led_service.pb.h"

#include <filesystem>
#include <fstream>

#include <gtest/gtest.h>

// Test subclass to access protected/private methods
class TestableLedManager : public hello_ipc::LedManager {
public:
    // Expose the private handler methods for direct testing
    using hello_ipc::LedManager::HandleUpdateRequest;
    using hello_ipc::LedManager::HandleQueryRequest;
    using hello_ipc::LedManager::UpdateLedState;
    using hello_ipc::LedManager::GetLedState;
};

class LedManagerTest : public ::testing::Test {
protected:
    std::string led_name = "999_test";
    std::string ledDir = "/tmp/sys/class/led_" + led_name;
    std::string filePath = ledDir + "/brightness";
    TestableLedManager manager;

    void SetUp() override {
        // Ensure the test directory is clean before each test
        std::filesystem::remove_all(ledDir);
    }

    void TearDown() override {
        // Clean up the test directory after each test
        std::filesystem::remove_all(ledDir);
    }
};

// --- Tests for UpdateLedState ---

TEST_F(LedManagerTest, UpdateLedStateReturnsTrueOnSuccess) {
    EXPECT_TRUE(manager.UpdateLedState(led_name, hello_ipc::LedState::ON));
    EXPECT_TRUE(std::filesystem::exists(filePath));
}

TEST_F(LedManagerTest, UpdateLedStateSetsCorrectState) {
    manager.UpdateLedState(led_name, hello_ipc::LedState::ON);
    std::ifstream file_on(filePath);
    std::string content_on;
    std::getline(file_on, content_on);
    EXPECT_EQ(content_on, "1");

    manager.UpdateLedState(led_name, hello_ipc::LedState::OFF);
    std::ifstream file_off(filePath);
    std::string content_off;
    std::getline(file_off, content_off);
    EXPECT_EQ(content_off, "0");
}

TEST_F(LedManagerTest, UpdateLedStateReturnsFalseForInvalidState) {
    // Test with an enum value that is not ON or OFF
    EXPECT_FALSE(manager.UpdateLedState(led_name, static_cast<hello_ipc::LedState>(123)));
    EXPECT_FALSE(std::filesystem::exists(filePath));
}

TEST_F(LedManagerTest, UpdateLedStateReturnsFalseForEmptyLedName) {
    EXPECT_FALSE(manager.UpdateLedState("", hello_ipc::LedState::ON));
}

// --- Tests for GetLedState ---

TEST_F(LedManagerTest, GetLedStateReturnsCorrectState) {
    std::filesystem::create_directories(ledDir);
    
    std::ofstream file_on(filePath);
    file_on << "1";
    file_on.close();
    EXPECT_EQ(manager.GetLedState(led_name), "on");

    std::ofstream file_off(filePath);
    file_off << "0";
    file_off.close();
    EXPECT_EQ(manager.GetLedState(led_name), "off");
}

TEST_F(LedManagerTest, GetLedStateReturnsErrorWhenNotFound) {
    EXPECT_EQ(manager.GetLedState("non_existent_led"), "error: LED not found");
}

TEST_F(LedManagerTest, GetLedStateReturnsErrorForEmptyLedName) {
    EXPECT_EQ(manager.GetLedState(""), "error: LED number cannot be empty");
}

// --- Tests for Protobuf Message Handlers ---

TEST_F(LedManagerTest, HandleUpdateRequestCorrectlyUpdatesState) {
    hello_ipc::LedUpdateRequest req;
    req.set_led_num(led_name);
    req.set_state(hello_ipc::LedState::ON);

    hello_ipc::LedStateResponse res;
    manager.HandleUpdateRequest(req, &res);

    // Check that the file system was updated
    EXPECT_TRUE(std::filesystem::exists(filePath));
    std::ifstream file(filePath);
    std::string content;
    std::getline(file, content);
    EXPECT_EQ(content, "1");

    // Check that the response message is populated correctly
    EXPECT_EQ(res.led_num(), led_name);
    EXPECT_EQ(res.state(), hello_ipc::LedState::ON);
    EXPECT_TRUE(res.error_message().empty());
}

TEST_F(LedManagerTest, HandleUpdateRequestHandlesFailure) {
    hello_ipc::LedUpdateRequest req;
    req.set_led_num(""); // Invalid LED name to force a failure
    req.set_state(hello_ipc::LedState::ON);

    hello_ipc::LedStateResponse res;
    manager.HandleUpdateRequest(req, &res);

    // Check that the response contains an error message
    EXPECT_EQ(res.led_num(), "");
    EXPECT_FALSE(res.error_message().empty());
}

TEST_F(LedManagerTest, HandleQueryRequestReturnsCorrectState) {
    // First, set a state to query
    manager.UpdateLedState(led_name, hello_ipc::LedState::ON);

    hello_ipc::LedQueryRequest req;
    req.set_led_num(led_name);

    hello_ipc::LedStateResponse res;
    manager.HandleQueryRequest(req, &res);

    // Check that the response is correct
    EXPECT_EQ(res.led_num(), led_name);
    EXPECT_EQ(res.state(), hello_ipc::LedState::ON);
    EXPECT_TRUE(res.error_message().empty());
}

TEST_F(LedManagerTest, HandleQueryRequestHandlesNotFound) {
    hello_ipc::LedQueryRequest req;
    req.set_led_num("non_existent_led");

    hello_ipc::LedStateResponse res;
    manager.HandleQueryRequest(req, &res);

    // Check that the response contains the error
    EXPECT_EQ(res.led_num(), "non_existent_led");
    EXPECT_EQ(res.error_message(), "error: LED not found");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}