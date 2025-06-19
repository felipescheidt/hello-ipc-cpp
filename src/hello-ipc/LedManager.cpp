#include <filesystem>
#include "LedManager.hpp"
#include "Logger.hpp"
#include <iostream>
#include <fstream>

/**
 * @file LedManager.cpp
 * @brief Implementation of the LedManager class to manage LED states via IPC.
 *
 * This class provides methods to update the state of an LED based on received messages.
 */
LedManager::LedManager(const std::string &ip, int port)
    : Service(ip, port, "LedManager") {
    logger_.log("LedManager initialized with IP: " + ip + " and port: " + std::to_string(port));
}

/**
 * @brief Updates the state of the LED based on the received message.
 *
 * This method parses the message to extract the LED name and state, creates the necessary directory
 * if it does not exist, and writes the state to a file named "brightness" in that directory.
 *
 * @param message The message containing the LED state to update in the format "ledNum=state".
 */
void LedManager::updateLedState(const std::string &message) {
    auto [ledNum, ledState] = Service::parseKeyValue(message);

    if (ledNum.empty() || ledState.empty()) {
        std::cerr << "Invalid message format: " << message << std::endl;
        return;
    }

    //if folder /tmp/sys/class/led_<ledNum> does not exist, create it
    std::string ledDir = "/tmp/sys/class/led_" + ledNum;
    if (!std::filesystem::exists(ledDir)) {
        try {
            std::filesystem::create_directories(ledDir);
            logger_.log("Created directory for LED: " + ledDir);
        } catch (const std::filesystem::filesystem_error &e) {
            std::cerr << "Error creating directory: " << e.what() << std::endl;
            return;
        }
    }
    std::string filePath = ledDir + "/brightness";
    std::ofstream ledFile(filePath);

    if (!ledFile.is_open()) {
        std::cerr << "Error opening file: " << filePath << std::endl;
        return;
    }

    ledFile << (ledState == "on" ? "1" : "0") << '\n';
    ledFile.close();

    logger_.log("Updated LED " + ledNum + " to state: " + ledState);
}
