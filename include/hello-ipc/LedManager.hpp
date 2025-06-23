#ifndef HELLO_IPC_LED_MANAGER_HPP_
#define HELLO_IPC_LED_MANAGER_HPP_

#include "Service.hpp"

/**
 * @file LedManager.hpp
 * @brief Class to manage LED states via IPC.
 *
 * This class provides methods to update the state of an LED based on received messages.
 * It handles the creation of directories and writing to files representing LED states.
 * 
 * @param socketPath The path to the socket file for communication.
 * @throws std::runtime_error if the socket connection fails or file operations fail.
 * @note This class is designed to run as a server, listening for incoming connections
 * and processing messages from clients.
 */
class LedManager : public Service {
    public:
        LedManager();
        void run(const std::string& socketPath);

    protected:
        void handleMessage(int client_socket, const std::string& message);
        void updateLedState(const std::string& ledNum, const std::string& ledState);
        std::string getLedState(const std::string& ledNum) const;
};

#endif // HELLO_IPC_LED_MANAGER_HPP_