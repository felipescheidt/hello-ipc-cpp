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
 * @param serviceName The name of the service for logging purposes.
 * @throws std::runtime_error if the socket connection fails or sending/receiving messages fails.
 */
class Service {
    public:
        explicit Service(const std::string &serviceName, bool connect = true);
        virtual ~Service();

        // For clients: sends a message.
        virtual void sendMessage(const std::string &message) const;

        // For clients: receives a message.
        virtual std::string receiveMessage();

        // Helper to parse "key=value" messages.
        static std::pair<std::string, std::string> parseKeyValue(const std::string &msg);

    protected:
        // For clients: connects to a server at a given socket path.
        void connectToServer(const std::string &socketPath);

        // For servers: runs a multi-threaded server loop.
        void runServer(const std::string &socketPath,
                    const std::function<void(int, const std::string&)>& messageHandler);

        // For servers: sends a response back to a specific client.
        void sendResponse(int client_socket, const std::string& message) const;

        Logger logger_;
        int sockfd;

    private:
        std::string receive_buffer_;
};

#endif // HELLO_IPC_SERVICE_HPP_