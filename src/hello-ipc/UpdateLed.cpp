/**
 * @file UpdateLed.cpp
 * @brief Implementation of the UpdateLed client for updating LED states via IPC.
 *
 * This file contains the implementation of methods for handling user input,
 * sending LED update commands, and processing command-line arguments.
 */

#include "UpdateLed.hpp"
#include <string>
#include <algorithm>
#include <cctype>


/** 
 * @brief Constructs an UpdateLed client that connects to the LedManager service.
 *
 * @param socketPath The path to the socket file for communication.
 * @param argc The argument count from the command line.
 * @param argv The argument vector from the command line.
 * @throws std::runtime_error if the socket connection fails.
 */
UpdateLed::UpdateLed(const std::string &socketPath, int argc, char** argv, bool connect)
    : Service("UpdateLed"), argc_(argc), argv_(argv) {
    if (connect) {
        connectToServer(socketPath);
    }
}

/** 
 * @brief Runs the UpdateLed client, processing command-line arguments and user input.
 *
 * This method handles command-line arguments to send initial LED updates,
 * and then enters a loop to handle user input for further updates.
 * 
 * @throws std::runtime_error if sending messages fails.
 */
void UpdateLed::run() {
    if (argc_ > 2) { // The first arg is the program, second is the mode
        handleArguments();
    }
    handleUserInput(std::cin);
}

/** 
 * @brief Handles command-line arguments to send initial LED updates.
 *
 * This method processes arguments of the form "--led<number>" to send updates
 * for LEDs specified in the command line.
 * 
 * @throws std::runtime_error if sending messages fails.
 */
void UpdateLed::handleArguments() {
    for (int i = 2; i < argc_; ++i) {
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
 * @brief Handles user input for LED updates.
 *
 * This method reads user input from the console, allowing users to specify
 * LED numbers and their states (on/off) interactively.
 *
 * @param inputStream The input stream to read from (e.g., std::cin).
 */
void UpdateLed::handleUserInput(std::istream &inputStream) {
    std::cout << "Welcome to the UpdateLed client!" << std::endl;
    std::cout << "Enter command ('1' for on, '!1' for off), or 'exit' to quit." << std::endl;
    std::string input;

    while (true) {
        if (&inputStream == &std::cin)
            std::cout << "> ";
        if (!std::getline(inputStream, input) || input == "exit")
            break;
        if (input.empty())
            continue;

        std::string ledName, ledState;
        if (input[0] == '!'){
            ledState = "off";
            ledName = input.substr(1);
        }  else {
            ledState = "on";
            ledName = input;
        }
        if (ledName.empty() || !std::all_of(ledName.begin(), ledName.end(), ::isdigit)) {
            std::cerr << "Invalid command." << std::endl;
            continue;
        }
        sendUpdate(ledName, ledState);
    }
}

/**
 * @brief Sends an update command for a specific LED.
 *
 * This method constructs a message in the format "ledName=ledState\n"
 * and sends it to the LedManager service.
 *
 * @param ledName The name of the LED to update.
 * @param ledState The new state of the LED ("on" or "off").
 * @throws std::runtime_error if sending the message fails.
 */
void UpdateLed::sendUpdate(const std::string &ledName, const std::string &ledState) {
    std::string message = ledName + "=" + ledState + "\n"; // ex: "1=on\n"
    sendMessage(message);
    logger_.log("Sent update for LED " + ledName + " to state: " + ledState);
}