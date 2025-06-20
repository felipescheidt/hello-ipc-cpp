#ifndef HELLO_IPC_UPDATE_LED_HPP_
#define HELLO_IPC_UPDATE_LED_HPP_
#include <iostream>
#include <vector>
#include <iostream>
#include <vector>
#include "Service.hpp"

/**
 * @brief A client application to send LED update commands via IPC.
 */
class UpdateLed : public Service {
    public:
        UpdateLed(const std::string &ip, int port, int argc, char **argv);
        void run();

    private:
        friend class UpdateLedTest;
        void handleArguments();
        void handleUserInput(std::istream& inputStream);
        void sendUpdate(const std::string &ledName, const std::string &ledState);

        int argc_;
        char **argv_;
};

#endif // HELLO_IPC_UPDATE_LED_HPP_