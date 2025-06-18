#include <filesystem>
#include "LedManager.hpp"
#include "Logger.hpp"

LedManager::LedManager(const std::string &ip, int port)
    : Service(ip, port, "LedManager") {
    logger_.log("LedManager initialized with IP: " + ip + " and port: " + std::to_string(port));
}

void LedManager::updateLedState(const std::string &message) {
    auto [ledName, ledState] = Service::parseKeyValue(message);

    if (ledName.empty() || ledState.empty()) {
        std::cerr << "Invalid message format: " << message << std::endl;
        return;
    }

    //if folder /tmp/sys/class/led_<ledName> does not exist, create it
    std::string ledDir = "/tmp/sys/class/led_" + ledName;
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

    ledFile << (ledState == "on" ? "1" : "0");
    ledFile.close();

    logger_.log("Updated LED " + ledName + " to state: " + ledState);
}
