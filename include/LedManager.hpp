#ifndef HELLO_IPC_LED_MANAGER_HPP_
#define HELLO_IPC_LED_MANAGER_HPP_
#include "Service.hpp"

/**
 * @file LedManager.hpp
 * @brief Class to manage LED states via IPC.
 *
 * This class provides methods to update the state of an LED based on received messages.
 */
class LedManager : public Service {
public:
    /**
     * @brief Constructs a LedManager object and establishes a TCP connection to the specified IP and port.
     * 
     * @param ip The IP address of the server.
     * @param port The port number of the server.
     * @throws std::runtime_error if socket creation, address conversion, or connection fails.
     */
    LedManager(const std::string &ip, int port);

    /**
     * @brief Updates the state of the LED based on the received message.
     * 
     * @param message The message containing the LED state to update.
     */
    void updateLedState(const std::string &message);

    /**
     * @brief Default constructor for LedManager, initializes with default IP and port.
     * This constructor is used for testing purposes.
     */
    LedManager() : Service("0.0.0.0", 0, "LedManager", true) {}
};

#endif // HELLO_IPC_LED_MANAGER_HPP_