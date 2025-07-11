#include "LedManager.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

namespace hello_ipc {

/**
 * @brief Constructs a LedManager service.
 */
LedManager::LedManager() : Service("LedManager", true) {}

/**
 * @brief Runs the LedManager server, listening for incoming connections.
 * 
 * @param socket_path Path to the socket file for communication.
 */
void LedManager::Run(const std::string &socket_path) {
    RunServer(socket_path, [this](int client_socket, const std::string &msg) {
        this->HandleMessage(client_socket, msg);
    });
}

/**
 * @brief Handles incoming messages from clients.
 * 
 * Parses the message and either queries or updates the LED state.
 * 
 * @param client_socket Socket file descriptor of the client.
 * @param message Received message from the client.
 */
void LedManager::HandleMessage(int client_socket, const std::string &message) {
    auto [key, value] = Service::ParseKeyValue(message);

    if (key == "QUERY") {
        logger().Log("Received query for LED: " + value);
        std::string state = GetLedState(value);
        std::string response = value + "=" + state + "\n";
        SendResponse(client_socket, response);
    } else {
        logger().Log("Received update for LED: " + key + " to state: " + value);
        UpdateLedState(key, value);
    }
}

/**
 * @brief Updates the state of an LED.
 * 
 * Creates necessary directories and writes the state to a file.
 * 
 * @param led_num LED number to update.
 * @param led_state New state of the LED ("on" or "off").
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
 * Reads the state from the file representing the LED's state.
 * 
 * @param led_num LED number to query.
 * @return State of the LED ("on" or "off"), or an error message if not found.
 */
std::string LedManager::GetLedState(const std::string &led_num) const {
    if (led_num.empty()) {
        return "error: LED number cannot be empty";
    }
    if (!std::filesystem::exists("/tmp/sys/class/led_" + led_num)) {
        return "error: LED not found";
    }
    std::string filePath = "/tmp/sys/class/led_" + led_num + "/brightness";
    std::ifstream ledFile(filePath);
    if (!ledFile.is_open())
        return "error";
    std::string state;
    std::getline(ledFile, state);

    return (state == "1") ? "on" : "off";
}

} // namespace hello_ipc