#ifndef HELLO_IPC_QUERY_LED_HPP_
#define HELLO_IPC_QUERY_LED_HPP_

#include "Service.hpp"
#include <iostream>

/**
 * @file QueryLed.hpp
 * @brief Class to query LED states via IPC.
 *
 * This class connects to the LedManager service and allows users to send queries
 * to retrieve the current state of LEDs.
 * 
 * @param socketPath The path to the socket file for communication.
 * @throws std::runtime_error if the socket connection fails or sending messages fails.
 */
class QueryLed : public Service {
    public:
        QueryLed(const std::string &socketPath, bool connect = true);
        void run();

    protected:
        void handleUserInput(std::istream &inputStream);
        void queryState(const std::string &ledName);
};

#endif // HELLO_IPC_QUERY_LED_HPP_