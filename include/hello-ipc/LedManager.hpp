#ifndef HELLO_IPC_LED_MANAGER_HPP_
#define HELLO_IPC_LED_MANAGER_HPP_

#include "Service.hpp"
#include "led_service.pb.h"

#include <string>
#include <functional>
namespace hello_ipc {

/**
 * @file LedManager.hpp
 * @brief Class to manage LED states via IPC.
 *
 * This class provides methods to update the state of an LED based on received messages.
 * It handles the creation of directories and writing to files representing LED states.
 * 
 * @param socket_path The path to the socket file for communication.
 * @throws std::runtime_error if the socket connection fails or file operations fail.
 * @note This class is designed to run as a server, listening for incoming connections
 * and processing messages from clients.
 */
class LedManager : public Service {
    public:
        LedManager();
        void Run(const std::string &socket_path);

    protected:
        void HandleMessage(int client_socket, const std::string &message);
        bool UpdateLedState(const std::string &led_num, hello_ipc::LedState led_state);
        void HandleUpdateRequest(const LedUpdateRequest &req, LedStateResponse *res);
        void HandleQueryRequest(const LedQueryRequest &req, LedStateResponse *res);
        std::string GetLedState(const std::string &led_num) const;
};

} // namespace hello_ipc

#endif // HELLO_IPC_LED_MANAGER_HPP_