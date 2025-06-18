#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include "LedManager.hpp"

class LedManagerTest : public ::testing::Test {
protected:
    std::string ledName = "123";
    std::string ledDir = "/tmp/sys/class/led_" + ledName;
    std::string filePath = ledDir + "/brightness";

    void SetUp() override {
        // Ensure the directory is clean before each test
        if (std::filesystem::exists(ledDir)) {
            std::filesystem::remove_all(ledDir);
        }
    }
};

TEST_F(LedManagerTest, InvalidMessageFormat) {
    LedManager manager;
    // Should not throw, just return
    EXPECT_NO_THROW(manager.updateLedState("invalidmessage"));
    EXPECT_FALSE(std::filesystem::exists(ledDir));
}

TEST_F(LedManagerTest, DirectoryIsCreated) {
    LedManager manager;
    std::string msg = ledName + "=on";
    manager.updateLedState(msg);
    EXPECT_TRUE(std::filesystem::exists(ledDir));
}

TEST_F(LedManagerTest, LedStateOnCreatesFileWithOne) {
    LedManager manager;
    std::string msg = ledName + "=on";
    manager.updateLedState(msg);
    std::ifstream file(filePath);
    std::string content;
    std::getline(file, content);
    EXPECT_EQ(content, "1");
}

TEST_F(LedManagerTest, LedStateOffCreatesFileWithZero) {
    LedManager manager;
    std::string msg = ledName + "=off";
    manager.updateLedState(msg);
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
    std::string msgOn = ledName + "=on";
    std::string msgOff = ledName + "=off";

    // Set LED to "on"
    manager.updateLedState(msgOn);
    {
        std::ifstream file(filePath);
        std::string content;
        std::getline(file, content);
        EXPECT_EQ(content, "1");
    }

    // Change LED to "off"
    manager.updateLedState(msgOff);
    {
        std::ifstream file(filePath);
        std::string content;
        std::getline(file, content);
        EXPECT_EQ(content, "0");
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}