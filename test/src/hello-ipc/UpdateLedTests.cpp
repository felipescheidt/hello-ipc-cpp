#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "hello-ipc/UpdateLed.hpp"

// A mock version of UpdateLed to intercept the sendMessage call for testing.
class MockUpdateLed : public UpdateLed {
public:
    // The constructor calls the base UpdateLed constructor.
    // We assume a 'testMode' flag exists to prevent real network connections.
    MockUpdateLed(int argc, char** argv)
        : UpdateLed("127.0.0.1", 12345, argc, argv, true) {}

    // Override the sendMessage method from the Service base class.
    MOCK_METHOD(void, sendMessage, (const std::string& message), (const, override));
};

// Test fixture for UpdateLed tests.
class UpdateLedTest : public ::testing::Test {
protected:
    // Helper to create a C-style argv array from a vector of strings.
    void create_argv(const std::vector<std::string>& args) {
        // Clear any previous state
        argv_storage_.clear();
        argv_ptr_.clear();

        // For each string argument...
        for (const auto& arg : args) {
            // ...create a null-terminated char array and store it.
            // The storage vector owns the memory.
            argv_storage_.emplace_back(arg.begin(), arg.end());
            argv_storage_.back().push_back('\0');

            // Store a pointer to the beginning of the char array.
            argv_ptr_.push_back(argv_storage_.back().data());
        }
    }

    // ... (CallHandleArguments helper and member variables are correct) ...
    void CallHandleArguments(UpdateLed& updateLed) {
        updateLed.handleArguments();
    }

    std::vector<std::vector<char>> argv_storage_;
    std::vector<char*> argv_ptr_;
};

// Test case: Verifies that handleArguments correctly parses a valid --led argument
TEST_F(UpdateLedTest, HandleArgumentsSendsUpdateForValidLedArg) {
    // Arrange
    create_argv({"./hello_ipc", "--led123"});
    MockUpdateLed mock_update_led(argv_ptr_.size(), argv_ptr_.data());

    // Expectation
    EXPECT_CALL(mock_update_led, sendMessage("123=on")).Times(1);

    // Act: Call the public helper in the fixture, which calls the private method.
    CallHandleArguments(mock_update_led);
}

// Test case: Verifies that handleArguments does nothing if no --led arguments are provided.
TEST_F(UpdateLedTest, HandleArgumentsIgnoresNonLedArgs) {
    // Arrange
    create_argv({"./hello_ipc", "--server", "some_other_arg"});
    MockUpdateLed mock_update_led(argv_ptr_.size(), argv_ptr_.data());

    // Expectation
    EXPECT_CALL(mock_update_led, sendMessage(::testing::_)).Times(0);

    // Act: Call the public helper.
    CallHandleArguments(mock_update_led);
}

// Test case: Verifies that handleArguments correctly handles multiple --led arguments.
TEST_F(UpdateLedTest, HandleArgumentsHandlesMultipleLedArgs) {
    // Arrange
    create_argv({"./hello_ipc", "--led1", "--other", "--led2"});
    MockUpdateLed mock_update_led(argv_ptr_.size(), argv_ptr_.data());

    // Expectation
    EXPECT_CALL(mock_update_led, sendMessage("1=on")).Times(1);
    EXPECT_CALL(mock_update_led, sendMessage("2=on")).Times(1);

    // Act: Call the public helper.
    CallHandleArguments(mock_update_led);
}

// Test case: Verifies that handleArguments ignores an argument where the led name is empty.
TEST_F(UpdateLedTest, HandleArgumentsIgnoresEmptyLedName) {
    // Arrange
    create_argv({"./hello_ipc", "--led"});
    MockUpdateLed mock_update_led(argv_ptr_.size(), argv_ptr_.data());

    // Expectation
    EXPECT_CALL(mock_update_led, sendMessage(::testing::_)).Times(0);

    // Act: Call the public helper.
    CallHandleArguments(mock_update_led);
}