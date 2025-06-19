#include "gtest/gtest.h"
#include "hello-ipc/Logger.hpp"
#include <filesystem>
#include <fstream>
#include <string>

/** @file LoggerTests.cpp
 * @brief Unit tests for the Logger class.
 *
 * This file contains unit tests for the Logger class, which is responsible for logging messages to a file.
 * The tests cover log file creation, message formatting, and appending messages.
 */
class LoggerTest : public ::testing::Test {
protected:
    void SetUp() override {
        serviceName = "TestLoggerService";
        logFilePath = "/tmp/" + serviceName + ".log";
        std::filesystem::remove(logFilePath);
    }

    void TearDown() override {
        std::filesystem::remove(logFilePath);
    }

    // Helper function to read the entire content of a file.
    std::string readFileContent(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            return "";
        }
        std::string content((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());
        return content;
    }

    std::string serviceName;
    std::string logFilePath;
};

TEST_F(LoggerTest, ConstructorCreatesLogFile) {
    {
        Logger logger(serviceName);
    }
    EXPECT_TRUE(std::filesystem::exists(logFilePath));
}

TEST_F(LoggerTest, LogMethodWritesCorrectFormat) {
    const std::string message = "This is a test message.";
    const std::string expectedContent = "[" + serviceName + "]: " + message + "\n";

    {
        Logger logger(serviceName);
        logger.log(message);
    } // Logger is destroyed here, closing the file.

    const std::string actualContent = readFileContent(logFilePath);
    EXPECT_EQ(actualContent, expectedContent);
}

TEST_F(LoggerTest, LogMethodAppendsMessages) {
    const std::string message1 = "First message.";
    const std::string message2 = "Second message.";
    const std::string expectedContent = "[" + serviceName + "]: " + message1 + "\n" +
                                        "[" + serviceName + "]: " + message2 + "\n";

    {
        Logger logger(serviceName);
        logger.log(message1);
        logger.log(message2);
    }

    const std::string actualContent = readFileContent(logFilePath);
    EXPECT_EQ(actualContent, expectedContent);
}

TEST_F(LoggerTest, HandlesEmptyLogMessage) {
    const std::string message = "";
    const std::string expectedContent = "[" + serviceName + "]: " + message + "\n";

    {
        Logger logger(serviceName);
        logger.log(message);
    }

    const std::string actualContent = readFileContent(logFilePath);
    EXPECT_EQ(actualContent, expectedContent);
}

TEST_F(LoggerTest, ConstructorThrowsOnInvalidPath) {
    // Arrange: Create a service name that results in an unwritable file path.
    const std::string invalidServiceName = "non_existent_dir/TestLogger";

    EXPECT_THROW({
        Logger logger(invalidServiceName);
    }, std::runtime_error);

    // Verify that our test helper returns an empty string for a non-existent file.
    EXPECT_EQ(readFileContent("/tmp/" + invalidServiceName + ".log"), "");
}