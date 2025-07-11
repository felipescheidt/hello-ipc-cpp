#include "QueryLed.hpp"

#include <sstream>
#include <vector>

#include <gtest/gtest.h>

// Test subclass to override send/receive for testing
class TestableQueryLed : public QueryLed {
    public:
        TestableQueryLed(const std::string& socket_path)
            : QueryLed(socket_path, false) {} // disables connection for tests

        using QueryLed::queryState;
        using QueryLed::HandleUserInput;

        std::vector<std::string> sentMessages;
        std::vector<std::string> responses;
        mutable int receiveCount = 0;

        void SendMessage(const std::string& message) const override {
            const_cast<TestableQueryLed*>(this)->sentMessages.push_back(message);
        }

        std::string ReceiveMessage() override {
            if (receiveCount < (int)responses.size())
                return responses[receiveCount++];
            return "mocked_response";
        }
};

TEST(QueryLedTest, QueryStateSendsCorrectMessageAndReceivesResponse) {
    TestableQueryLed client("/tmp/fake.sock");
    client.responses = {"on"};
    testing::internal::CaptureStdout();
    client.queryState("1");
    std::string output = testing::internal::GetCapturedStdout();

    ASSERT_EQ(client.sentMessages.size(), 1);
    EXPECT_EQ(client.sentMessages[0], "QUERY=1\n");
    EXPECT_NE(output.find("Response: on"), std::string::npos);
}

TEST(QueryLedTest, HandleUserInputQueriesAndExits) {
    TestableQueryLed client("/tmp/fake.sock");
    client.responses = {"off"};
    std::istringstream input("2\nexit\n");
    testing::internal::CaptureStdout();
    client.HandleUserInput(input);
    std::string output = testing::internal::GetCapturedStdout();

    ASSERT_EQ(client.sentMessages.size(), 1);
    EXPECT_EQ(client.sentMessages[0], "QUERY=2\n");
    EXPECT_NE(output.find("Response: off"), std::string::npos);
}

TEST(QueryLedTest, HandleUserInputRejectsInvalidInput) {
    TestableQueryLed client("/tmp/fake.sock");
    std::istringstream input("abc\n\nexit\n");
    testing::internal::CaptureStderr();
    client.HandleUserInput(input);
    std::string err = testing::internal::GetCapturedStderr();

    EXPECT_TRUE(client.sentMessages.empty());
    EXPECT_NE(err.find("Invalid input"), std::string::npos);
}

TEST(QueryLedTest, QueryStateHandlesReceiveException) {
    class ExceptionQueryLed : public TestableQueryLed {
    public:
        using TestableQueryLed::TestableQueryLed;
        std::string ReceiveMessage() override {
            throw std::runtime_error("fail");
        }
    };
    ExceptionQueryLed client("/tmp/fake.sock");
    testing::internal::CaptureStdout();
    client.queryState("3");
    std::string output = testing::internal::GetCapturedStdout();
    // Should not throw, should log error
    EXPECT_NE(output.find("Error receiving response"), std::string::npos);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}