#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "hello-ipc/UpdateLed.hpp"
#include <sstream>

/** * @file UpdateLedTests.cpp
 * @brief Unit tests for the UpdateLed class.
 *
 * This file contains unit tests for the UpdateLed class, which is responsible for updating LED states via IPC.
 * The tests cover command-line argument parsing, user input handling, and the sendUpdate method.
 */
class MockUpdateLed : public UpdateLed {
public:
    // The constructor calls the base UpdateLed constructor in test mode.
    MockUpdateLed(int argc, char** argv)
        : UpdateLed("127.0.0.1", 12345, argc, argv, true) {}

    // Override the sendMessage method from the Service base class.
    MOCK_METHOD(void, sendMessage, (const std::string& message), (const, override));
};

/** * @brief Test fixture for the UpdateLed class.
 * 
 * This fixture sets up the necessary environment for testing the UpdateLed class,
 * including creating command-line arguments and providing helper methods to call private methods.
 */
class UpdateLedTest : public ::testing::Test {
protected:
    void create_argv(const std::vector<std::string>& args) {
        argv_storage_.clear();
        argv_ptr_.clear();
        for (const auto& arg : args) {
            argv_storage_.emplace_back(arg.begin(), arg.end());
            argv_storage_.back().push_back('\0');
            argv_ptr_.push_back(argv_storage_.back().data());
        }
    }

    // Helper to call the private handleArguments method
    void CallHandleArguments(UpdateLed& updateLed) {
        updateLed.handleArguments();
    }

    // Helper to call the private handleUserInput method
    void CallHandleUserInput(UpdateLed& updateLed, std::istream& inputStream) {
        updateLed.handleUserInput(inputStream);
    }

    // Helper to call the private sendUpdate method
    void CallSendUpdate(UpdateLed& updateLed, const std::string& ledName, const std::string& ledState) {
        updateLed.sendUpdate(ledName, ledState);
    }

    std::vector<std::vector<char>> argv_storage_;
    std::vector<char*> argv_ptr_;
};


TEST_F(UpdateLedTest, HandleArgumentsSendsUpdateForValidLedArg) {
    create_argv({"./hello_ipc", "--led123"});
    MockUpdateLed mock_update_led(argv_ptr_.size(), argv_ptr_.data());

    EXPECT_CALL(mock_update_led, sendMessage("123=on\n")).Times(1);

    CallHandleArguments(mock_update_led);
}

TEST_F(UpdateLedTest, HandleArgumentsIgnoresNonLedArgs) {
    create_argv({"./hello_ipc", "--server", "some_other_arg"});
    MockUpdateLed mock_update_led(argv_ptr_.size(), argv_ptr_.data());

    EXPECT_CALL(mock_update_led, sendMessage(::testing::_)).Times(0);

    CallHandleArguments(mock_update_led);
}

TEST_F(UpdateLedTest, HandleArgumentsHandlesMultipleLedArgs) {
    create_argv({"./hello_ipc", "--led1", "--other", "--led2"});
    MockUpdateLed mock_update_led(argv_ptr_.size(), argv_ptr_.data());

    EXPECT_CALL(mock_update_led, sendMessage("1=on\n")).Times(1);
    EXPECT_CALL(mock_update_led, sendMessage("2=on\n")).Times(1);

    CallHandleArguments(mock_update_led);
}

TEST_F(UpdateLedTest, HandleArgumentsIgnoresEmptyLedName) {
    create_argv({"./hello_ipc", "--led"});
    MockUpdateLed mock_update_led(argv_ptr_.size(), argv_ptr_.data());

    EXPECT_CALL(mock_update_led, sendMessage(::testing::_)).Times(0);

    CallHandleArguments(mock_update_led);
}

TEST_F(UpdateLedTest, HandleUserInputHappyPath) {
    create_argv({"./hello_ipc"});
    MockUpdateLed mock_update_led(argv_ptr_.size(), argv_ptr_.data());
    std::stringstream fake_input("1\n!2\nexit\n");

    // Expect calls in a specific order
    ::testing::InSequence s;
    EXPECT_CALL(mock_update_led, sendMessage("1=on\n")).Times(1);
    EXPECT_CALL(mock_update_led, sendMessage("2=off\n")).Times(1);

    CallHandleUserInput(mock_update_led, fake_input);
}

TEST_F(UpdateLedTest, HandleUserInputInvalidCommands) {
    create_argv({"./hello_ipc"});
    MockUpdateLed mock_update_led(argv_ptr_.size(), argv_ptr_.data());
    std::stringstream fake_input("\nnot_a_number\n!\n!abc\nexit\n");

    EXPECT_CALL(mock_update_led, sendMessage(::testing::_)).Times(0);

    CallHandleUserInput(mock_update_led, fake_input);
}

TEST_F(UpdateLedTest, SendUpdateRejectsInvalidParameters) {
    create_argv({"./hello_ipc"});
    MockUpdateLed mock_update_led(argv_ptr_.size(), argv_ptr_.data());

    EXPECT_CALL(mock_update_led, sendMessage(::testing::_)).Times(0);

    CallSendUpdate(mock_update_led, "", "on");
    CallSendUpdate(mock_update_led, "1", "invalid_state");
}