#include "Service.hpp"

#include <stdexcept>
#include <cstring>
#include <iostream>
#include <thread>
#include <vector>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

namespace hello_ipc {

/** 
 * @brief Constructs a Service with the given service name.
 *
 * Initializes the logger and sets the socket file descriptor to -1 (not connected).
 *
 * @param service_name The name of the service for logging purposes.
 */
Service::Service(const std::string &service_name, bool connect)
            : logger_(service_name), sockfd_(-1) {
    (void)connect; // Suppress unused parameter warning
}

/** 
 * @brief Destructor closes the socket if it is open.
 */
Service::~Service() {
    if (sockfd_ >= 0) {
        close(sockfd_);
    }
}

/** 
 * @brief Connects to a server at the specified socket path.
 *
 * @param socket_path The path to the socket file.
 * @throws std::runtime_error if the connection fails.
 */
void Service::ConnectToServer(const std::string &socket_path) {
    sockfd_ = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd_ < 0) throw std::runtime_error("Failed to create socket");

    struct sockaddr_un server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, socket_path.c_str(), sizeof(server_addr.sun_path) - 1);

    if (connect(sockfd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        throw std::runtime_error("Connection failed to " + socket_path);
    }

    logger().Log("Connection established to " + socket_path);
}

/** 
 * @brief Runs a multi-threaded server loop that listens for incoming connections.
 *
 * @param socket_path The path to the socket file.
 * @param message_handler A function to handle incoming messages from clients.
 */
void Service::RunServer(const std::string &socket_path,
                        const std::function<void(int, const std::string&)> &message_handler) {

    int server_fd = CreateServerSocket(socket_path);
    logger().Log("Server listening on socket: " + socket_path);

    while (true) {
        int client_socket = accept(server_fd, NULL, NULL);
        if (client_socket < 0) {
            perror("accept");
            continue;
        }

        logger().Log("Accepted new connection.");
        std::cout << "New client connected. ID: " << client_socket << std::endl;

        // Spawn a new thread to handle the client
        std::thread([this, client_socket, handler = std::move(message_handler)]() {
            std::string buffer;
            char read_buffer[1024];
            while (true) {
                ssize_t received = recv(client_socket, read_buffer, sizeof(read_buffer) - 1, 0);
                if (received <= 0) break;

                buffer.append(read_buffer, received);
                size_t pos;
                while ((pos = buffer.find('\n')) != std::string::npos) {
                    std::string message = buffer.substr(0, pos);
                    buffer.erase(0, pos + 1);
                    if (!message.empty()) {
                        handler(client_socket, message);
                    }
                }
            }
            logger().Log("Client disconnected.");
            std::cout << "Client disconnected. ID: " << client_socket << std::endl;
            close(client_socket);
        }).detach();
    }
}

/** 
 * @brief Sends a message to the connected server.
 *
 * @param message The message to send.
 * @throws std::runtime_error if the socket is not connected or sending fails.
 */
void Service::SendMessage(const std::string &message) const {
    if (sockfd_ < 0) {
        throw std::runtime_error("Not connected");
    }
    if (send(sockfd_, message.c_str(), message.length(), 0) < 0) {
        throw std::runtime_error("Failed to send message");
    }
}

/** 
 * @brief Sends a response back to a specific client.
 *
 * @param client_socket The socket file descriptor of the client.
 * @param message The message to send as a response.
 * @throws std::runtime_error if sending fails.
 */
void Service::SendResponse(int client_socket, const std::string &message) const {
    if (send(client_socket, message.c_str(), message.length(), 0) < 0) {
        logger().Log("Failed to send response to client.");
        throw std::runtime_error("Failed to send response");
    }
}

/** 
 * @brief Receives a message from the connected server.
 *
 * @return The received message.
 * @throws std::runtime_error if the connection is closed or receiving fails.
 */
std::string Service::ReceiveMessage() {
    if (sockfd_ < 0) {
        throw std::runtime_error("Socket is not connected.");
    }

    // Set socket timeout
    SetupSocketTimeout(sockfd_);

    char read_buffer[1024] = {0};

    while (receive_buffer_.find('\n') == std::string::npos) {
        ssize_t bytes_received = recv(sockfd_, read_buffer, sizeof(read_buffer) - 1, 0);
        if (bytes_received < 0) {
            throw std::runtime_error("Error receiving data from server.");
        } else if (bytes_received == 0) {
            throw std::runtime_error("Connection closed by server.");
        }
        receive_buffer_.append(read_buffer, bytes_received);
    }

    size_t pos = receive_buffer_.find('\n');
    std::string message = receive_buffer_.substr(0, pos);
    receive_buffer_.erase(0, pos + 1);

    return message;
}

/** 
 * @brief Parses a "key=value" message into a key-value pair.
 *
 * @param msg The message to parse.
 * @return A pair containing the key and value.
 */
std::pair<std::string, std::string> Service::ParseKeyValue(const std::string &msg) {
    size_t pos = msg.find('=');
    if (pos == std::string::npos) {
        return {msg, ""};
    }

    return {msg.substr(0, pos), msg.substr(pos + 1)};
}

/** 
 * @brief Sets up the socket with necessary options.
 *
 * Configures the socket to have a timeout for receiving and sending data.
 *
 * @param sockfd The socket file descriptor to configure.
 * @throws std::runtime_error if setting socket options fails.
 */
void Service::SetupSocketTimeout(int sockfd) const {
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    // Set the socket receive timeout
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        throw std::runtime_error("Failed to set socket receive timeout");
    }

    // Set the socket send timeout
    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
        throw std::runtime_error("Failed to set socket send timeout");
    }
}

/** 
 * @brief Creates a server socket and binds it to the specified path.
 *
 * @param socket_path The path to bind the server socket.
 * @return The file descriptor of the created server socket.
 * @throws std::runtime_error if socket creation, binding, or listening fails.
 */
int Service::CreateServerSocket(const std::string &socket_path) const {
    // Ensure the socket file does not already exist
    unlink(socket_path.c_str());

    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        throw std::runtime_error("Failed to create server socket");
    }

    struct sockaddr_un address;
    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, socket_path.c_str(), sizeof(address.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        close(server_fd);
        throw std::runtime_error("Failed to bind server socket to " + socket_path);
    }

    if (listen(server_fd, 10) < 0) {
        close(server_fd);
        throw std::runtime_error("Failed to listen on server socket");
    }

    return server_fd;
}

} // namespace hello_ipc