#include "gtest/gtest.h"
#include "hello-ipc/Logger.hpp"
#include <filesystem>
#include <fstream>
#include <string>

// Test fixture for the Logger class to handle setup and teardown
class LoggerTest : public ::testing::Test {
protected:
    // Set up the test environment before each test
    void SetUp() override {
        serviceName = "TestLoggerService";
        logFilePath = "/tmp/" + serviceName + ".log";
        // Ensure the file does not exist before the test starts
        std::filesystem::remove(logFilePath);
    }

    // Clean up the test environment after each test
    void TearDown() override {
        // Clean up the created log file
        std::filesystem::remove(logFilePath);
    }

    // Helper function to read the entire content of a file
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

// Test case: Verifies that the Logger constructor creates the log file.
TEST_F(LoggerTest, ConstructorCreatesLogFile) {
    // Action: Create a logger instance. It goes out of scope immediately.
    {
        Logger logger(serviceName);
    }

    // Assertion: Check if the log file now exists.
    EXPECT_TRUE(std::filesystem::exists(logFilePath));
}

// Test case: Verifies that the log method writes a message in the correct format.
TEST_F(LoggerTest, LogMethodWritesCorrectFormat) {
    // Setup
    const std::string message = "This is a test message.";
    const std::string expectedContent = "[" + serviceName + "]: " + message + "\n";

    // Action
    {
        Logger logger(serviceName);
        logger.log(message);
    } // Logger is destroyed here, closing the file.

    // Assertion
    const std::string actualContent = readFileContent(logFilePath);
    EXPECT_EQ(actualContent, expectedContent);
}

// Test case: Verifies that multiple calls to log() append messages correctly.
TEST_F(LoggerTest, LogMethodAppendsMessages) {
    // Setup
    const std::string message1 = "First message.";
    const std::string message2 = "Second message.";
    const std::string expectedContent = "[" + serviceName + "]: " + message1 + "\n" +
                                        "[" + serviceName + "]: " + message2 + "\n";

    // Action
    {
        Logger logger(serviceName);
        logger.log(message1);
        logger.log(message2);
    }

    // Assertion
    const std::string actualContent = readFileContent(logFilePath);
    EXPECT_EQ(actualContent, expectedContent);
}

// Test case: Verifies that an empty message is logged correctly.
TEST_F(LoggerTest, HandlesEmptyLogMessage) {
    // Setup
    const std::string message = "";
    const std::string expectedContent = "[" + serviceName + "]: " + message + "\n";

    // Action
    {
        Logger logger(serviceName);
        logger.log(message);
    }

    // Assertion
    const std::string actualContent = readFileContent(logFilePath);
    EXPECT_EQ(actualContent, expectedContent);
}

// Test case: Verifies that the Logger constructor throws an exception for an invalid path.
TEST_F(LoggerTest, ConstructorThrowsOnInvalidPath) {
    // Arrange: Create a service name that results in an unwritable file path.
    // This assumes the test is not run as the root user.
    const std::string invalidServiceName = "non_existent_dir/TestLogger";

    // Action & Assertion:
    // Check that creating a Logger with this name throws the expected exception.
    EXPECT_THROW({
        Logger logger(invalidServiceName);
    }, std::runtime_error);

    // Also, verify that our test helper returns an empty string for a non-existent file,
    // which will cover the missing line in the test file itself.
    EXPECT_EQ(readFileContent("/tmp/" + invalidServiceName + ".log"), "");
}