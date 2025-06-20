#include "LedManager.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>

/** * @file LedManager.cpp
 * @brief Implementation of the LedManager service that manages LED states.
 *
 * This class connects to a message broker and listens for LED state update commands.
 * It updates the LED state by writing to the appropriate file in the /tmp/sys/class/led_<led_num> directory.
 */
LedManager::LedManager() : Service("LedManager") {}


/**
 * @brief Runs the LedManager service.
 *
 * This method connects to the message broker and listens for LED state update commands.
 * It updates the LED state by writing to the appropriate file in the /tmp/sys/class/led_<led_num> directory.
 *
 * @param ip The IP address of the message broker.
 * @param port The port number of the message broker.
 */
void LedManager::run(const std::string& ip, int port) {
    connectToServer(ip, port);
    logger_.log("Connected to broker. Registering as subscriber.");

    // Announce to the broker that we are a subscriber
    sendMessage("SUBSCRIBE\n");

    logger_.log("Subscription successful. Waiting for commands...");
    while (true) {
        try {
            std::string message = receiveMessage();
            updateLedState(message);
        } catch (const std::runtime_error& e) {
            logger_.log("Disconnected from broker: " + std::string(e.what()));
            break;
        }
    }
}

/**
 * @brief Updates the LED state based on the received message.
 *
 * This method parses the message to extract the LED number and state,
 * then writes the state to the appropriate file in the /tmp/sys/class/led_<led_num> directory.
 *
 * @param message The message containing the LED number and state in the format "led_num=state".
 */
void LedManager::updateLedState(const std::string &message) {
    auto [ledNum, ledState] = Service::parseKeyValue(message);
    if (ledNum.empty() || ledState.empty()) {
        std::cerr << "Invalid message format: " << message << std::endl;
        return;
    }
    // ... (rest of the file writing logic is the same) ...
    std::string ledDir = "/tmp/sys/class/led_" + ledNum;
    try {
        if (!std::filesystem::exists(ledDir)) {
            std::filesystem::create_directories(ledDir);
        }
    } catch (const std::filesystem::filesystem_error &e) {
        std::cerr << "Failed to create directory: " << ledDir << " - " << e.what() << std::endl;
        return;
    }
    std::string filePath = ledDir + "/brightness";
    std::ofstream ledFile(filePath);
    if (!ledFile.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return;
    }
    ledFile << (ledState == "on" ? "1" : "0") << '\n';
    logger_.log("Processed command. Updated LED " + ledNum + " to state: " + ledState);
}