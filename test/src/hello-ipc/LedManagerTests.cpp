#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include "LedManager.hpp"

/** @file LedManagerTests.cpp
 * @brief Unit tests for the LedManager class.
 *
 * This file contains unit tests for the LedManager class, which is responsible for managing LED states via IPC.
 * The tests cover directory creation, file writing, and error handling.
 */
class LedManagerTest : public ::testing::Test {
protected:
    std::string ledName = "123";
    std::string ledDir = "/tmp/sys/class/led_" + ledName;
    std::string filePath = ledDir + "/brightness";

    void SetUp() override {
        if (std::filesystem::exists(ledDir)) {
            std::filesystem::remove_all(ledDir);
        }
    }

    void TearDown() override {
        // Ensure permissions are restored for cleanup
        if (std::filesystem::exists(ledDir)) {
            std::filesystem::permissions(ledDir, std::filesystem::perms::owner_all);
            std::filesystem::remove_all(ledDir);
        }
    }
};

TEST_F(LedManagerTest, InvalidMessageFormat) {
    LedManager manager;
    EXPECT_NO_THROW(manager.updateLedState("invalidmessage"));
    EXPECT_FALSE(std::filesystem::exists(ledDir));
}

TEST_F(LedManagerTest, DirectoryIsCreated) {
    LedManager manager;
    manager.updateLedState(ledName + "=on");
    EXPECT_TRUE(std::filesystem::exists(ledDir));
}

TEST_F(LedManagerTest, LedStateOnCreatesFileWithOne) {
    LedManager manager;
    manager.updateLedState(ledName + "=on");
    std::ifstream file(filePath);
    std::string content;
    std::getline(file, content);
    EXPECT_EQ(content, "1");
}

TEST_F(LedManagerTest, LedStateOffCreatesFileWithZero) {
    LedManager manager;
    manager.updateLedState(ledName + "=off");
    std::ifstream file(filePath);
    std::string content;
    std::getline(file, content);
    EXPECT_EQ(content, "0");
}

TEST_F(LedManagerTest, EmptyLedNameOrState) {
    LedManager manager;
    EXPECT_NO_THROW(manager.updateLedState("="));
    EXPECT_NO_THROW(manager.updateLedState("led="));
    EXPECT_NO_THROW(manager.updateLedState("=on"));
    EXPECT_FALSE(std::filesystem::exists(ledDir));
}

TEST_F(LedManagerTest, LedStateChangeUpdatesFile) {
    LedManager manager;
    manager.updateLedState(ledName + "=on");
    {
        std::ifstream file(filePath);
        std::string content;
        std::getline(file, content);
        EXPECT_EQ(content, "1");
    }
    manager.updateLedState(ledName + "=off");
    {
        std::ifstream file(filePath);
        std::string content;
        std::getline(file, content);
        EXPECT_EQ(content, "0");
    }
}

TEST_F(LedManagerTest, HandlesDirectoryCreationError) {
    std::string parentDir = "/tmp/sys/class";
    std::filesystem::create_directories(parentDir);
    std::ofstream(ledDir);
    
    LedManager manager;
    EXPECT_NO_THROW(manager.updateLedState(ledName + "=on"));
}

TEST_F(LedManagerTest, HandlesFileOpenError) {
    std::filesystem::create_directories(ledDir);
    std::filesystem::permissions(ledDir, std::filesystem::perms::owner_read | std::filesystem::perms::owner_exec);

    LedManager manager;
    EXPECT_NO_THROW(manager.updateLedState(ledName + "=on"));
    EXPECT_FALSE(std::filesystem::exists(filePath));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}