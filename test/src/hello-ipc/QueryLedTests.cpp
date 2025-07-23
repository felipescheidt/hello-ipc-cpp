#include "QueryLed.hpp"
#include "led_service.pb.h"

#include <sstream>
#include <vector>
#include <optional>

#include <gtest/gtest.h>

// Test subclass to override send/receive for testing
class TestableQueryLed : public hello_ipc::QueryLed {
public:
    TestableQueryLed()
        : hello_ipc::QueryLed("/tmp/fake.sock", false) {} // disables connection for tests

    using hello_ipc::QueryLed::queryState;
    using hello_ipc::QueryLed::HandleUserInput;

    std::vector<std::string> sentMessages;
    std::vector<std::optional<std::string>> responses;
    mutable int receiveCount = 0;

    // Override SendMessage to capture the serialized protobuf message
    void SendMessage(const std::string& message) const override {
        const_cast<TestableQueryLed*>(this)->sentMessages.push_back(message);
    }

    // Override ReceiveMessage to provide mocked responses
    std::optional<std::string> ReceiveMessage() override {
        if (receiveCount < (int)responses.size()) {
            return responses[receiveCount++];
        }
        return std::nullopt; // Default to no response
    }
};

TEST(QueryLedTest, QueryStateSendsCorrectProtobufMessage) {
    TestableQueryLed client;
    client.queryState("1");

    // Verify that one message was "sent"
    ASSERT_EQ(client.sentMessages.size(), 1);

    // Parse the sent message to verify its contents
    hello_ipc::Request sent_req;
    ASSERT_TRUE(sent_req.ParseFromString(client.sentMessages[0]));
    ASSERT_TRUE(sent_req.has_query_request());
    EXPECT_EQ(sent_req.query_request().led_num(), "1");
}

TEST(QueryLedTest, QueryStatePrintsCorrectResponse) {
    TestableQueryLed client;

    // Create and serialize a mock response
    hello_ipc::Response mock_res;
    auto* state_res = mock_res.mutable_state_response();
    state_res->set_led_num("1");
    state_res->set_state(hello_ipc::LedState::ON);
    
    std::string response_str;
    mock_res.SerializeToString(&response_str);
    client.responses.push_back(response_str);

    // Capture stdout and run the query
    testing::internal::CaptureStdout();
    client.queryState("1");
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("Response: Led1=on"), std::string::npos);
}

TEST(QueryLedTest, QueryStateHandlesServerErrorResponse) {
    TestableQueryLed client;

    // Create and serialize a mock error response
    hello_ipc::Response mock_res;
    auto* state_res = mock_res.mutable_state_response();
    state_res->set_error_message("LED is on fire");

    std::string response_str;
    mock_res.SerializeToString(&response_str);
    client.responses.push_back(response_str);

    testing::internal::CaptureStdout();
    client.queryState("1");
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("Error from server: LED is on fire"), std::string::npos);
}

TEST(QueryLedTest, QueryStateHandlesReceiveFailure) {
    TestableQueryLed client;
    client.responses.push_back(std::nullopt); // Simulate a receive failure

    testing::internal::CaptureStderr();
    client.queryState("3");
    std::string err_output = testing::internal::GetCapturedStderr();
    
    EXPECT_NE(err_output.find("Error: Failed to receive response from server."), std::string::npos);
}

TEST(QueryLedTest, HandleUserInputRejectsInvalidInput) {
    TestableQueryLed client;
    std::istringstream input("abc\n\nexit\n");
    
    testing::internal::CaptureStderr();
    client.HandleUserInput(input);
    std::string err = testing::internal::GetCapturedStderr();

    EXPECT_TRUE(client.sentMessages.empty());
    EXPECT_NE(err.find("Invalid input. LED name must be a number."), std::string::npos);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}