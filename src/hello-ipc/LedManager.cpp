/** @file LedManager.cpp
 * @brief Implementation of the LedManager class for managing LED states via IPC.
 *
 * This file contains the implementation of methods for handling incoming messages,
 * updating LED states, and querying LED states.
 */

#include "LedManager.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>


/** 
 * @brief Constructs a LedManager service.
 *
 * Initializes the base Service class with the service name "LedManager".
 */
LedManager::LedManager() : Service("LedManager", true) {}

/** 
 * @brief Runs the LedManager server, listening for incoming connections.
 *
 * @param socketPath The path to the socket file for communication.
 * @throws std::runtime_error if the socket connection fails or file operations fail.
 */
void LedManager::run(const std::string &socketPath) {
    // Use the generic server runner from the base class
    runServer(socketPath, [this](int client_socket, const std::string &msg) {
        this->handleMessage(client_socket, msg);
    });
}

/** 
 * @brief Handles incoming messages from clients.
 *
 * This method parses the message and updates the LED state or queries the LED state
 * based on the received command.
 *
 * @param client_socket The socket file descriptor of the client.
 * @param message The received message from the client.
 */
void LedManager::handleMessage(int client_socket, const std::string &message) {
    auto [key, value] = Service::parseKeyValue(message);

    if (key == "QUERY") {
        logger_.log("Received query for LED: " + value);
        std::string state = getLedState(value);
        std::string response = value + "=" + state + "\n";
        sendResponse(client_socket, response);
    } else {
        // Assume it's an update message (e.g., "1=on")
        logger_.log("Received update for LED: " + key + " to state: " + value);
        updateLedState(key, value);
    }
}

/** 
 * @brief Updates the state of an LED based on the received message.
 *
 * This method creates the necessary directories and writes the state to a file
 * representing the LED's state.
 *
 * @param ledNum The number of the LED to update.
 * @param ledState The new state of the LED ("on" or "off").
 */
void LedManager::updateLedState(const std::string &ledNum, const std::string &ledState) {
    if (ledNum.empty() || ledState.empty()) {
        logger_.log("Invalid update format.");
        return;
    }
    if (ledState != "on" && ledState != "off") {
        logger_.log("Invalid LED state: " + ledState);
        return;
    }
    std::string ledDir = "/tmp/sys/class/led_" + ledNum;
    std::string filePath = ledDir + "/brightness";
    try {
        if (!std::filesystem::exists(ledDir)) {
            std::filesystem::create_directories(ledDir);
        }
        std::ofstream ledFile(filePath);
        ledFile.exceptions(std::ofstream::failbit | std::ofstream::badbit);
        ledFile << (ledState == "on" ? "1" : "0") << '\n';
        logger_.log("Updated LED " + ledNum + " to state: " + ledState);
    } catch (const std::exception &e) {
        logger_.log("Error writing to file " + filePath + ": " + e.what());
    }
}

/** 
 * @brief Gets the current state of an LED.
 *
 * This method reads the state from the file representing the LED's state.
 *
 * @param ledNum The number of the LED to query.
 * @return The state of the LED ("on" or "off"), or an error message if the LED is not found.
 */
std::string LedManager::getLedState(const std::string &ledNum) const {
    if (ledNum.empty()) {
        return "error: LED number cannot be empty";
    }
    if (!std::filesystem::exists("/tmp/sys/class/led_" + ledNum)) {
        return "error: LED not found";
    }
    // Read the state from the file
    std::string filePath = "/tmp/sys/class/led_" + ledNum + "/brightness";
    std::ifstream ledFile(filePath);
    if (!ledFile.is_open())
        return "error";
    std::string state;
    std::getline(ledFile, state);

    return (state == "1") ? "on" : "off";
}