#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include "LedManager.hpp"

// Test subclass to access protected methods
class TestableLedManager : public LedManager {
    public:
        using LedManager::HandleMessage;
        using LedManager::UpdateLedState;
        using LedManager::GetLedState;
};

class LedManagerTest : public ::testing::Test {
protected:
    std::string ledName = "999_test";
    std::string ledDir = "/tmp/sys/class/led_" + ledName;
    std::string filePath = ledDir + "/brightness";
    TestableLedManager manager;

    void SetUp() override {
        std::filesystem::remove_all(ledDir);
    }

    void TearDown() override {
        if (std::filesystem::exists(ledDir)) {
            try {
                std::filesystem::permissions(ledDir, std::filesystem::perms::owner_all,
                                             std::filesystem::perm_options::replace);
            } catch (...) {
                // Ignore errors if directory doesn't exist or permissions can't be set
            }
            std::filesystem::remove_all(ledDir);
        }
    }
};

TEST_F(LedManagerTest, UpdateLedStateCreatesDirectoryAndFile) {
    manager.UpdateLedState(ledName, "on");
    EXPECT_TRUE(std::filesystem::exists(ledDir));
    EXPECT_TRUE(std::filesystem::exists(filePath));
}

TEST_F(LedManagerTest, UpdateLedStateSetsCorrectState) {
    manager.UpdateLedState(ledName, "on");
    std::ifstream file_on(filePath);
    std::string content_on;
    std::getline(file_on, content_on);
    EXPECT_EQ(content_on, "1");

    manager.UpdateLedState(ledName, "off");
    std::ifstream file_off(filePath);
    std::string content_off;
    std::getline(file_off, content_off);
    EXPECT_EQ(content_off, "0");
}

TEST_F(LedManagerTest, UpdateLedStateRejectsInvalidState) {
    manager.UpdateLedState(ledName, "blinking");
    EXPECT_FALSE(std::filesystem::exists(filePath));
}

TEST_F(LedManagerTest, GetLedStateReturnsCorrectState) {
    std::filesystem::create_directories(ledDir);
    std::ofstream file(filePath);
    file << "1";
    file.close();
    EXPECT_EQ(manager.GetLedState(ledName), "on");

    std::ofstream file2(filePath);
    file2 << "0";
    file2.close();
    EXPECT_EQ(manager.GetLedState(ledName), "off");
}

TEST_F(LedManagerTest, GetLedStateReturnsErrorWhenNotFound) {
    EXPECT_EQ(manager.GetLedState(ledName), "error: LED not found");
    EXPECT_EQ(manager.GetLedState(""), "error: LED number cannot be empty");
}

TEST_F(LedManagerTest, HandleMessageUpdateCreatesFile) {
    manager.HandleMessage(-1, ledName + "=on");
    EXPECT_TRUE(std::filesystem::exists(filePath));
    std::ifstream file(filePath);
    std::string content;
    std::getline(file, content);
    EXPECT_EQ(content, "1");
}

TEST_F(LedManagerTest, HandleMessageQueryThrowsOnInvalidSocket) {
    manager.UpdateLedState(ledName, "on");
    // Now expect an exception, since sending on socket -1 throws
    EXPECT_THROW(manager.HandleMessage(-1, "QUERY=" + ledName), std::runtime_error);
}

TEST_F(LedManagerTest, HandleMessageIgnoresInvalidFormats) {
    manager.HandleMessage(-1, "=");
    manager.HandleMessage(-1, ledName + "=");
    manager.HandleMessage(-1, "=on");
    manager.HandleMessage(-1, ledName + "=blinking");
    EXPECT_FALSE(std::filesystem::exists(filePath));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}