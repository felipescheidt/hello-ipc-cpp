#ifndef HELLO_IPC_SERVICE_HPP_
#define HELLO_IPC_SERVICE_HPP_

#include "Logger.hpp"

#include <string>
#include <functional>
#include <optional>

namespace hello_ipc {

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
        virtual std::optional<std::string> ReceiveMessage();

    protected:
        // For clients: connects to a server at a given socket path.
        bool ConnectToServer(const std::string &socket_path);

        // For servers: runs a multi-threaded server loop.
        void RunServer(const std::string &socket_path,
                    const std::function<void(int, const std::string&)>& message_handler);

        // For servers: sends a response back to a specific client.
        void SendResponse(int client_socket, const std::string& message) const;

        // For servers: creates a server socket and binds it to the specified path.
        int CreateServerSocket(const std::string &socket_path) const;

        const Logger &logger() const { return logger_; }

        // Getter for sockfd_
        void SetSocketFd(int fd) { sockfd_ = fd; }
        const int& GetSocket() const { return sockfd_; }

    private:
        void SetupSocketTimeout(int sockfd) const;
        Logger logger_;
        int sockfd_;
        std::string receive_buffer_;
};

} // namespace hello_ipc

#endif // HELLO_IPC_SERVICE_HPP_