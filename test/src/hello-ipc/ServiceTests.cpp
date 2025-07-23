#include "Service.hpp"

#include <gtest/gtest.h>

// Test subclass to expose protected methods for testing
class TestableService : public hello_ipc::Service {
public:
    TestableService(const std::string& name)
        : hello_ipc::Service(name, false) {} // Always disable auto-connect for tests

    // Expose protected methods for testing
    using hello_ipc::Service::ConnectToServer;
    using hello_ipc::Service::SetSocketFd;
};

// --- Unit Tests for Service Class ---
// These tests do not require a live server and focus on error handling paths.

TEST(ServiceTest, ConnectToServerFailsForInvalidPath) {
    TestableService svc("svc");
    // ConnectToServer should fail gracefully and return false, not throw or crash.
    EXPECT_FALSE(svc.ConnectToServer("/tmp/this_socket_should_not_exist_12345.sock"));
}

TEST(ServiceTest, SendMessageFailsIfSocketInvalid) {
    TestableService svc("svc");
    svc.SetSocketFd(-1); // Use the protected setter to create an invalid state

    // SendMessage should handle the invalid socket gracefully without crashing.
    // The underlying send() will fail, which is logged by the Service class.
    svc.SendMessage("hello");
    // The test passes if no crash occurs.
}

TEST(ServiceTest, ReceiveMessageFailsIfSocketInvalid) {
    TestableService svc("svc");
    svc.SetSocketFd(-1); // Use the protected setter to create an invalid state

    // ReceiveMessage should return an empty optional, not throw or crash.
    auto result = svc.ReceiveMessage();
    EXPECT_FALSE(result.has_value());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}