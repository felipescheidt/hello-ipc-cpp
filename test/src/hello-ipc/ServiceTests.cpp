#include <gtest/gtest.h>
#include "Service.hpp"

/** * @file ServiceTests.cpp
 * @brief Unit tests for the Service class.
 *
 * This file contains unit tests for the Service class, including message parsing and connection handling.
 */
class TestService : public Service {
public:
    TestService(const std::string &ip, int port) : Service(ip, port, "TestService", false) {}
};

TEST(ServiceTest, ParseKeyValue_ValidInput) {
    std::string msg = "value=key";
    auto result = Service::parseKeyValue(msg);
    EXPECT_EQ(result.first, "value");
    EXPECT_EQ(result.second, "key");
}

TEST(ServiceTest, ParseKeyValue_NoValue) {
    std::string msg = "value=";
    auto result = Service::parseKeyValue(msg);
    EXPECT_EQ(result.first, "value");
    EXPECT_EQ(result.second, "");
}

TEST(ServiceTest, ParseKeyValue_EmptyString) {
    auto result = Service::parseKeyValue("");
    EXPECT_EQ(result.first, "");
    EXPECT_EQ(result.second, "");
}

TEST(ServiceTest, ParseKeyValue_NoEquals) {
    std::string msg = "invalidmessage";
    auto result = Service::parseKeyValue(msg);
    EXPECT_EQ(result.first, "invalidmessage");
    EXPECT_EQ(result.second, "");
}

TEST(ServiceTest, ConnectionFailure) {
    EXPECT_THROW(TestService("256.256.256.256", 12345), std::runtime_error);
}

TEST(ServiceTest, InvalidIP) {
    EXPECT_THROW(TestService("invalid_ip", 12345), std::runtime_error);
}

TEST(ServiceTest, SendAndReceiveMessageWithInvalidPort) {
    EXPECT_THROW(TestService("127.0.0.1", 99999), std::runtime_error);
}

TEST(ServiceTest, DISABLED_SendAndReceiveMessage) {
    TestService service("127.0.0.1", 12345);
    std::string testMsg = "ping=test";
    EXPECT_NO_THROW(service.sendMessage(testMsg));
    // The server should respond with "pong=test"
    std::string response = service.receiveMessage();
    EXPECT_EQ(response, "pong=test\n");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}