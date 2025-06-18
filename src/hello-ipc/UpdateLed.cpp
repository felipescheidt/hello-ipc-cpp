#include "UpdateLed.hpp"
#include "Logger.hpp"
#include "LedManager.hpp"
#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>

/** * @file UpdateLed.cpp
 * @brief Implementation of the UpdateLed client that interacts with the LedManager service.
 *
 * This class handles command-line arguments to activate LEDs and provides an interactive loop
 * for user input to update LED states.
 */
UpdateLed::UpdateLed(const std::string& ip, int port, int argc, char** argv, bool testMode)
    : Service(ip, port, "UpdateLed", testMode), argc_(argc), argv_(argv) {
    logger_.log("UpdateLed client initialized with IP: " + ip + " and port: " + std::to_string(port));
}

/**
 * @brief Runs the UpdateLed client.
 *
 * This method processes command-line arguments to activate LEDs and enters an interactive loop
 * for user input to update LED states.
 */
void UpdateLed::run() {
    if (argc_ > 1) {
        handleArguments();
    }
    handleUserInput();
}

/**
 * @brief Handles command-line arguments to activate LEDs specified by --led<number>.
 *
 * This method checks each argument for the pattern "--led<number>" and sends an update
 * to turn on the corresponding LED.
 */
void UpdateLed::handleArguments() {
    // Loop through all arguments to activate LEDs specified by --led<number>
    for (int i = 1; i < argc_; ++i) {
        std::string arg = argv_[i];
        if (arg.rfind("--led", 0) == 0) { // Check if argument starts with --led
            std::string ledName = arg.substr(5); // Extract the number after "--led"
            if (!ledName.empty()) {
                sendUpdate(ledName, "on");
            }
        }
    }
}

/**
 * @brief Handles user input to update LED states interactively.
 *
 * This method prompts the user for input to turn LEDs on or off by entering commands
 * like '1' for on, '!1' for off, or 'exit' to quit the loop.
 */
void UpdateLed::handleUserInput() {
    std::cout << "Welcome to the UpdateLed client!" << std::endl;
    std::cout << "Usage:" << std::endl;
    std::cout << "  '1' for led1=on" << std::endl;
    std::cout << " '!1' for led1=off" << std::endl;
    std::cout << "  'exit' to quit" << std::endl;

    std::string input;
    while (true) {
        std::cout << "Enter command: ";
        std::getline(std::cin, input);

        if (input == "exit") {
            break;
        }

        if (input.empty()) {
            continue;
        }

        std::string ledName;
        std::string ledState;

        if (input[0] == '!') {
            ledState = "off";
            ledName = input.substr(1);
        } else {
            ledState = "on";
            ledName = input;
        }

        if (ledName.empty()) {
            std::cerr << "Invalid command: LED name cannot be empty." << std::endl;
            continue;
        }

        if (!std::all_of(ledName.begin(), ledName.end(), ::isdigit)) {
            std::cerr << "Invalid command: LED name must be a number." << std::endl;
            continue;
        }

        sendUpdate(ledName, ledState);
    }
}

/**
 * @brief Sends an update to the LED state.
 *
 * This method constructs a message in the format "ledName=ledState" and sends it
 * to the LedManager service. It logs the action taken.
 *
 * @param ledName The name of the LED to update.
 * @param ledState The desired state of the LED ("on" or "off").
 */
void UpdateLed::sendUpdate(const std::string& ledName, const std::string& ledState) {
    if (ledName.empty() || (ledState != "on" && ledState != "off")) {
        std::cerr << "Invalid LED name or state." << std::endl;
        return;
    }

    std::string message = ledName + "=" + ledState;
    sendMessage(message);
    logger_.log("Sent update for LED " + ledName + " to state: " + ledState);
}
