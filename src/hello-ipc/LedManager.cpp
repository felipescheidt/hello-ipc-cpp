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
 * @param socket_path The path to the socket file for communication.
 * @throws std::runtime_error if the socket connection fails or file operations fail.
 */
void LedManager::Run(const std::string &socket_path) {
    // Use the generic server runner from the base class
    RunServer(socket_path, [this](int client_socket, const std::string &msg) {
        this->HandleMessage(client_socket, msg);
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
void LedManager::HandleMessage(int client_socket, const std::string &message) {
    auto [key, value] = Service::ParseKeyValue(message);

    if (key == "QUERY") {
        logger().Log("Received query for LED: " + value);
        std::string state = GetLedState(value);
        std::string response = value + "=" + state + "\n";
        SendResponse(client_socket, response);
    } else {
        // Assume it's an update message (e.g., "1=on")
        logger().Log("Received update for LED: " + key + " to state: " + value);
        UpdateLedState(key, value);
    }
}

/** 
 * @brief Updates the state of an LED based on the received message.
 *
 * This method creates the necessary directories and writes the state to a file
 * representing the LED's state.
 *
 * @param led_num The number of the LED to update.
 * @param led_state The new state of the LED ("on" or "off").
 */
void LedManager::UpdateLedState(const std::string &led_num, const std::string &led_state) {
    if (led_num.empty() || led_state.empty()) {
        logger().Log("Invalid update format.");
        return;
    }
    if (led_state != "on" && led_state != "off") {
        logger().Log("Invalid LED state: " + led_state);
        return;
    }
    std::string ledDir = "/tmp/sys/class/led_" + led_num;
    std::string filePath = ledDir + "/brightness";
    try {
        if (!std::filesystem::exists(ledDir)) {
            std::filesystem::create_directories(ledDir);
        }
        std::ofstream ledFile(filePath);
        ledFile.exceptions(std::ofstream::failbit | std::ofstream::badbit);
        ledFile << (led_state == "on" ? "1" : "0") << '\n';
        logger().Log("Updated LED " + led_num + " to state: " + led_state);
    } catch (const std::exception &e) {
        logger().Log("Error writing to file " + filePath + ": " + e.what());
    }
}

/** 
 * @brief Gets the current state of an LED.
 *
 * This method reads the state from the file representing the LED's state.
 *
 * @param led_num The number of the LED to query.
 * @return The state of the LED ("on" or "off"), or an error message if the LED is not found.
 */
std::string LedManager::GetLedState(const std::string &led_num) const {
    if (led_num.empty()) {
        return "error: LED number cannot be empty";
    }
    if (!std::filesystem::exists("/tmp/sys/class/led_" + led_num)) {
        return "error: LED not found";
    }
    // Read the state from the file
    std::string filePath = "/tmp/sys/class/led_" + led_num + "/brightness";
    std::ifstream ledFile(filePath);
    if (!ledFile.is_open())
        return "error";
    std::string state;
    std::getline(ledFile, state);

    return (state == "1") ? "on" : "off";
}