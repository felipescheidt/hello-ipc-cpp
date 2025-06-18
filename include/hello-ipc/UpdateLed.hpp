#ifndef HELLO_IPC_UPDATE_LED_HPP_
#define HELLO_IPC_UPDATE_LED_HPP_
#include "Service.hpp"
#include "LedManager.hpp"
#include <iostream>
#include <vector>

/**
 * @file UpdateLed.hpp
 * @brief Class to update LED states via IPC.
 *
 * This class handles command-line arguments to activate LEDs and provides an interactive loop
 * for user input to update LED states.
 */
class UpdateLed : public Service {
    public:
        UpdateLed(const std::string &ip, int port, int argc, char **argv, bool testMode = false);

        void run();

    private:
        friend class UpdateLedTest;
        void handleArguments();
        void handleUserInput();
        void sendUpdate(const std::string &ledName, const std::string &ledState);

        int argc_;
        char **argv_;
};

#endif // HELLO_IPC_UPDATE_LED_HPP_