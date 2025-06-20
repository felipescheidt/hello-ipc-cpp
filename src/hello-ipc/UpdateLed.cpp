#include "UpdateLed.hpp"
#include <string>
#include <algorithm>
#include <cctype>

/** * @file UpdateLed.cpp
 * @brief Implementation of the UpdateLed client application.
 *
 * This class connects to a message broker and allows users to send LED update commands.
 * It can also handle command-line arguments for initial LED states.
 */
UpdateLed::UpdateLed(const std::string& ip, int port, int argc, char** argv)
    : Service("UpdateLed"), argc_(argc), argv_(argv) {
    // The base constructor is called, then we explicitly connect.
    connectToServer(ip, port);
    logger_.log("UpdateLed client connected to " + ip + ":" + std::to_string(port));
}

/**
 * @brief Runs the UpdateLed client application.
 *
 * This method processes command-line arguments and handles user input for LED updates.
 */
void UpdateLed::run() {
    if (argc_ > 1) {
        handleArguments();
    }
    handleUserInput(std::cin);
}

/**
 * @brief Handles command-line arguments to send initial LED updates.
 *
 * This method checks for arguments of the form --led<number> and sends an "on" update for each.
 */
void UpdateLed::handleArguments() {
    for (int i = 1; i < argc_; ++i) {
        std::string arg = argv_[i];
        if (arg.rfind("--led", 0) == 0) {
            std::string ledName = arg.substr(5);
            if (!ledName.empty()) {
                sendUpdate(ledName, "on");
            }
        }
    }
}

/**
 * @brief Handles user input from the console to send LED updates.
 *
 * This method prompts the user for commands to turn LEDs on or off.
 * Commands can be of the form "1" for on, "!1" for off, or "exit" to quit.
 */
void UpdateLed::handleUserInput(std::istream& inputStream) {
    std::cout << "Welcome to the UpdateLed client!" << std::endl;
    std::cout << "Enter command ('1' for on, '!1' for off, 'exit' to quit):" << std::endl;

    logger_.log("User input mode started. Waiting for commands...");

    std::string input;
    while (true) {
        if (&inputStream == &std::cin) {
            std::cout << "> ";
        }
        if (!std::getline(inputStream, input) || input == "exit") {
            break;
        }
        if (input.empty()) continue;

        std::string ledName;
        std::string ledState;

        if (input[0] == '!') {
            ledState = "off";
            ledName = input.substr(1);
        } else {
            ledState = "on";
            ledName = input;
        }

        if (ledName.empty() || !std::all_of(ledName.begin(), ledName.end(), ::isdigit)) {
            std::cerr << "Invalid command. LED name must be a non-empty number." << std::endl;
            continue;
        }
        sendUpdate(ledName, ledState);
    }
}

/**
 * @brief Sends an LED update command to the broker.
 *
 * This method constructs a message in the format "ledName=ledState\n" and sends it.
 * It logs the sent message for debugging purposes.
 *
 * @param ledName The name of the LED (e.g., "1", "2").
 * @param ledState The state of the LED ("on" or "off").
 */
void UpdateLed::sendUpdate(const std::string& ledName, const std::string& ledState) {
    if (ledName.empty() || (ledState != "on" && ledState != "off")) {
        std::cerr << "Invalid LED name or state." << std::endl;
        return;
    }
    std::string message = ledName + "=" + ledState + "\n";
    sendMessage(message);
    logger_.log("Sent update: " + message);
}