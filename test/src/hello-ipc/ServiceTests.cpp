#include <gtest/gtest.h>
#include "Service.hpp"

// A simple derived class to instantiate Service for testing
class TestService : public Service {
public:
    TestService(const std::string& ip, int port) : Service(ip, port) {}
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

TEST(ServiceTest, DISABLED_SendAndReceiveMessage) {
    // Replace with a valid IP and port where a test server is running
    TestService service("127.0.0.1", 12345);
    std::string testMsg = "ping=test";
    EXPECT_NO_THROW(service.sendMessage(testMsg));
    // The server should respond with "pong=test"
    std::string response = service.receiveMessage();
    EXPECT_EQ(response, "pong=test\n");
}
