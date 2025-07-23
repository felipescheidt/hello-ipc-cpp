#include "UpdateLed.hpp"
#include "led_service.pb.h"

#include <sstream>
#include <vector>
#include <optional>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

// Test subclass to override send/receive for testing
class TestableUpdateLed : public hello_ipc::UpdateLed {
public:
    // The constructor passes 'false' for the 'connect' parameter to prevent real connection attempts
    TestableUpdateLed(int argc, char** argv)
        : hello_ipc::UpdateLed("/tmp/fake.sock", argc, argv, false) {}

    using hello_ipc::UpdateLed::HandleArguments;
    using hello_ipc::UpdateLed::HandleUserInput;
    using hello_ipc::UpdateLed::SendUpdate;

    std::vector<std::string> sentMessages;
    std::vector<std::optional<std::string>> responses;
    mutable int receiveCount = 0;

    // Override SendMessage to capture the serialized protobuf message
    void SendMessage(const std::string& message) const override {
        const_cast<TestableUpdateLed*>(this)->sentMessages.push_back(message);
    }

    // Override ReceiveMessage to provide mocked responses
    std::optional<std::string> ReceiveMessage() override {
        if (receiveCount < (int)responses.size()) {
            return responses[receiveCount++];
        }
        return std::nullopt; // Default to no response
    }
};

// Helper function to create a mock response
std::string CreateMockResponse(const std::string& led_num, hello_ipc::LedState state, const std::string& error = "") {
    hello_ipc::Response res;
    auto* state_res = res.mutable_state_response();
    state_res->set_led_num(led_num);
    if (error.empty()) {
        state_res->set_state(state);
    } else {
        state_res->set_error_message(error);
    }
    std::string response_str;
    res.SerializeToString(&response_str);
    return response_str;
}

TEST(UpdateLedTest, HandleArgumentsSendsCorrectProtobuf) {
    const char* argv[] = {"prog", "--update-led", "--led1", "--led2"};
    TestableUpdateLed updater(4, const_cast<char**>(argv));
    
    // Provide mock responses for the two sends
    updater.responses.push_back(CreateMockResponse("1", hello_ipc::LedState::ON));
    updater.responses.push_back(CreateMockResponse("2", hello_ipc::LedState::ON));

    updater.HandleArguments();
    ASSERT_EQ(updater.sentMessages.size(), 2);

    // Verify first message
    hello_ipc::Request req1;
    ASSERT_TRUE(req1.ParseFromString(updater.sentMessages[0]));
    ASSERT_TRUE(req1.has_update_request());
    EXPECT_EQ(req1.update_request().led_num(), "1");
    EXPECT_EQ(req1.update_request().state(), hello_ipc::LedState::ON);

    // Verify second message
    hello_ipc::Request req2;
    ASSERT_TRUE(req2.ParseFromString(updater.sentMessages[1]));
    ASSERT_TRUE(req2.has_update_request());
    EXPECT_EQ(req2.update_request().led_num(), "2");
    EXPECT_EQ(req2.update_request().state(), hello_ipc::LedState::ON);
}

TEST(UpdateLedTest, HandleUserInputSendsOnAndOff) {
    const char* argv[] = {"prog", "--update-led"};
    TestableUpdateLed updater(2, const_cast<char**>(argv));
    std::istringstream input("1\n!2\nexit\n");

    // Provide mock responses
    updater.responses.push_back(CreateMockResponse("1", hello_ipc::LedState::ON));
    updater.responses.push_back(CreateMockResponse("2", hello_ipc::LedState::OFF));

    updater.HandleUserInput(input);
    ASSERT_EQ(updater.sentMessages.size(), 2);

    // Verify ON message
    hello_ipc::Request req_on;
    ASSERT_TRUE(req_on.ParseFromString(updater.sentMessages[0]));
    EXPECT_EQ(req_on.update_request().led_num(), "1");
    EXPECT_EQ(req_on.update_request().state(), hello_ipc::LedState::ON);

    // Verify OFF message
    hello_ipc::Request req_off;
    ASSERT_TRUE(req_off.ParseFromString(updater.sentMessages[1]));
    EXPECT_EQ(req_off.update_request().led_num(), "2");
    EXPECT_EQ(req_off.update_request().state(), hello_ipc::LedState::OFF);
}

TEST(UpdateLedTest, SendUpdateHandlesServerError) {
    const char* argv[] = {"prog", "--update-led"};
    TestableUpdateLed updater(2, const_cast<char**>(argv));
    
    // Provide a mock error response
    updater.responses.push_back(CreateMockResponse("3", hello_ipc::LedState::ON, "System is on fire"));

    testing::internal::CaptureStdout();
    updater.SendUpdate("3", "on");
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_THAT(output, testing::HasSubstr("Error from server: System is on fire"));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}