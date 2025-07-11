#ifndef HELLO_IPC_SERVICE_HPP_
#define HELLO_IPC_SERVICE_HPP_

#include <string>
#include <functional>
#include "Logger.hpp"

/** 
 * @file Service.hpp
 * @brief Base class for IPC services using TCP/IP sockets.
 *
 * This class provides methods for sending and receiving messages, parsing key-value pairs,
 * and managing the socket connection.
 * 
 * @param service_name The name of the service for logging purposes.
 * @throws std::runtime_error if the socket connection fails or sending/receiving messages fails.
 */
class Service {
    public:
        explicit Service(const std::string &service_name, bool connect = true);
        virtual ~Service();

        // For clients: sends a message.
        virtual void SendMessage(const std::string &message) const;

        // For clients: receives a message.
        virtual std::string ReceiveMessage();

        // Helper to parse "key=value" messages.
        static std::pair<std::string, std::string> ParseKeyValue(const std::string &msg);

    protected:
        // For clients: connects to a server at a given socket path.
        void ConnectToServer(const std::string &socket_path);

        // For servers: runs a multi-threaded server loop.
        void RunServer(const std::string &socket_path,
                    const std::function<void(int, const std::string&)>& message_handler);

        // For servers: sends a response back to a specific client.
        void SendResponse(int client_socket, const std::string& message) const;

        Logger logger_;
        int sockfd_;

    private:
        std::string receive_buffer_;
};

#endif // HELLO_IPC_SERVICE_HPP_