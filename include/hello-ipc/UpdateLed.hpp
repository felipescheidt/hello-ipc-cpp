#ifndef HELLO_IPC_UPDATE_LED_HPP_
#define HELLO_IPC_UPDATE_LED_HPP_

#include "Service.hpp"
#include <iostream>

/**
 * @file UpdateLed.hpp
 * @brief A client to update the state of LEDs via IPC.
 *
 * This class connects to the LedManager service and allows users to send commands
 * to turn LEDs on or off based on user input or command-line arguments.
 * 
 * @param socketPath The path to the socket file for communication.
 * @param argc The argument count from the command line.
 * @param argv The argument vector from the command line.
 * @throws std::runtime_error if the socket connection fails or sending messages fails.
 */
class UpdateLed : public Service {
    public:
        UpdateLed(const std::string &socketPath, int argc, char** argv, bool connect = true);
        void run();

    protected:
        void handleArguments();
        void handleUserInput(std::istream &inputStream);
        void sendUpdate(const std::string &ledName, const std::string &ledState);

        int argc_;
        char** argv_;
};

#endif // HELLO_IPC_UPDATE_LED_HPP_