#include <gtest/gtest.h>
#include "Service.hpp"
#include <stdexcept>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <thread>
#include <atomic>
#include <chrono>

// Test subclass to expose/provide hooks for testing
class TestableService : public Service {
    public:
        TestableService(const std::string& name, bool connect = false)
            : Service(name, connect) {}
        using Service::ConnectToServer;
        using Service::SendResponse;
        using Service::ReceiveMessage;
        using Service::SendMessage;
        using Service::RunServer;
        int& sock() { return sockfd_; }
};

TEST(ServiceTest, RunServerReceivesMessage) {
    const std::string socket_path = "/tmp/test_service_RunServer.sock";
    std::atomic<bool> messageReceived{false};
    std::string receivedMsg;
    int receivedClient = -1;

    // Start server in a background thread
    std::thread serverThread([&] {
        TestableService svc("svc", false);
        svc.RunServer(socket_path, [&](int client, const std::string& msg) {
            receivedMsg = msg;
            receivedClient = client;
            messageReceived = true;
            // Optionally, send a response or close the socket here
        });
    });

    // Give the server a moment to start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Connect as a client and send a message
    int clientSock = socket(AF_UNIX, SOCK_STREAM, 0);
    ASSERT_GE(clientSock, 0);

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path.c_str(), sizeof(addr.sun_path) - 1);

    ASSERT_EQ(connect(clientSock, (struct sockaddr*)&addr, sizeof(addr)), 0);

    const std::string testMsg = "hello_from_client\n";
    ASSERT_EQ(send(clientSock, testMsg.c_str(), testMsg.size(), 0), (ssize_t)testMsg.size());

    // Wait for the server to receive the message
    for (int i = 0; i < 20 && !messageReceived; ++i)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    EXPECT_TRUE(messageReceived);
    EXPECT_EQ(receivedMsg, "hello_from_client");

    // Cleanup
    close(clientSock);
    unlink(socket_path.c_str());
    // The server thread is still running; in a real test, you'd signal it to stop.
    // For this minimal test, just detach to let the process exit.
    serverThread.detach();
}

TEST(ServiceTest, ParseKeyValueReturnsKeyAndValue) {
    auto result = Service::ParseKeyValue("led1=on");
    EXPECT_EQ(result.first, "led1");
    EXPECT_EQ(result.second, "on");
}

TEST(ServiceTest, ParseKeyValueHandlesNoEquals) {
    auto result = Service::ParseKeyValue("led1");
    EXPECT_EQ(result.first, "led1");
    EXPECT_EQ(result.second, "");
}

TEST(ServiceTest, ParseKeyValueHandlesEmptyString) {
    auto result = Service::ParseKeyValue("");
    EXPECT_EQ(result.first, "");
    EXPECT_EQ(result.second, "");
}

TEST(ServiceTest, ParseKeyValueHandlesMultipleEquals) {
    auto result = Service::ParseKeyValue("foo=bar=baz");
    EXPECT_EQ(result.first, "foo");
    EXPECT_EQ(result.second, "bar=baz");
}

TEST(ServiceTest, ConnectToServerThrowsOnInvalidPath) {
    TestableService svc("svc", false);
    EXPECT_THROW(svc.ConnectToServer("/tmp/this_socket_should_not_exist.sock"), std::runtime_error);
}

TEST(ServiceTest, SendMessageThrowsIfSocketInvalid) {
    TestableService svc("svc", false);
    svc.sock() = -1; // force invalid socket
    EXPECT_THROW(svc.SendMessage("hello"), std::runtime_error);
}

TEST(ServiceTest, ReceiveMessageThrowsIfSocketInvalid) {
    TestableService svc("svc", false);
    svc.sock() = -1; // force invalid socket
    EXPECT_THROW({
        svc.ReceiveMessage();
    }, std::runtime_error);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}