#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "hello-ipc/UpdateLed.hpp"
#include <sstream>

// Test subclass to expose protected/private methods for testing
class TestableUpdateLed : public UpdateLed {
    public:
        TestableUpdateLed(const std::string &socket_path, int argc, char** argv)
            : UpdateLed(socket_path, argc, argv, false) {}

        using UpdateLed::HandleArguments;
        using UpdateLed::HandleUserInput;
        using UpdateLed::SendUpdate;

        std::vector<std::string> sentMessages;
        void SendMessage(const std::string& message) const override {
            const_cast<TestableUpdateLed*>(this)->sentMessages.push_back(message);
        }
};

TEST(UpdateLedTest, HandleArgumentsSendsCorrectUpdates) {
    const char* argv[] = {"prog", "--update-led", "--led1", "--led2"};
    TestableUpdateLed updater("/tmp/fake.sock", 4, const_cast<char**>(argv));
    updater.HandleArguments();
    ASSERT_EQ(updater.sentMessages.size(), 2);
    EXPECT_THAT(updater.sentMessages[0], testing::HasSubstr("1=on"));
    EXPECT_THAT(updater.sentMessages[1], testing::HasSubstr("2=on"));
}

TEST(UpdateLedTest, HandleUserInputSendsOnAndOff) {
    const char* argv[] = {"prog", "--update-led"};
    TestableUpdateLed updater("/tmp/fake.sock", 2, const_cast<char**>(argv));
    std::istringstream input("1\n!2\nexit\n");
    updater.HandleUserInput(input);
    ASSERT_EQ(updater.sentMessages.size(), 2);
    EXPECT_THAT(updater.sentMessages[0], testing::HasSubstr("1=on"));
    EXPECT_THAT(updater.sentMessages[1], testing::HasSubstr("2=off"));
}

TEST(UpdateLedTest, HandleUserInputIgnoresInvalidCommands) {
    const char* argv[] = {"prog", "--update-led"};
    TestableUpdateLed updater("/tmp/fake.sock", 2, const_cast<char**>(argv));
    std::istringstream input("abc\n!xyz\n\nexit\n");
    updater.HandleUserInput(input);
    EXPECT_TRUE(updater.sentMessages.empty());
}

TEST(UpdateLedTest, SendUpdateFormatsMessageCorrectly) {
    const char* argv[] = {"prog", "--update-led"};
    TestableUpdateLed updater("/tmp/fake.sock", 2, const_cast<char**>(argv));
    updater.SendUpdate("3", "on");
    ASSERT_EQ(updater.sentMessages.size(), 1);
    EXPECT_EQ(updater.sentMessages[0], "3=on\n");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}