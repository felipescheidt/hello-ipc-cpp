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
        LedManager(const std::string &ip, int port);
        void updateLedState(const std::string &message);

        /**
         * @brief Default constructor for LedManager, initializes with default IP and port.
         * This constructor is used for testing purposes.
         */
        LedManager() : Service("0.0.0.0", 0, "LedManager", true) {}
};

#endif // HELLO_IPC_LED_MANAGER_HPP_