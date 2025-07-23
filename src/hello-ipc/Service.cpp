#include "Service.hpp"

#include <stdexcept>
#include <cstring>
#include <iostream>
#include <thread>
#include <vector>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>

namespace hello_ipc {

const int kMaxMessageSize = 4096;

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
 * @return true if connection is successful, false otherwise.
 */
bool Service::ConnectToServer(const std::string &socket_path) {
    sockfd_ = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd_ < 0) {
        logger().Log("Error creating socket: " + std::string(strerror(errno)));
        return false;
    }

    struct sockaddr_un server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, socket_path.c_str(), sizeof(server_addr.sun_path) - 1);

    if (connect(sockfd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        logger().Log("Error connecting to server: " + std::string(strerror(errno)));
        close(sockfd_);
        sockfd_ = -1;
        return false;
    }

    logger().Log("Connection established to " + socket_path);
    return true;
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
        std::cout << "New client connected. ID= " << client_socket << std::endl;

        // Spawn a new thread to handle the client
        std::thread([this, client_socket, handler = std::move(message_handler)]() {
            while (true) {
                uint32_t msg_size;
                // Read the 4-byte size prefix
                ssize_t received = recv(client_socket, &msg_size, sizeof(msg_size), MSG_WAITALL);
                if (received <= 0) {
                    break; // Client disconnected or error
                }

                msg_size = ntohl(msg_size); // Convert from network to host byte order
                if (msg_size == 0 || msg_size > kMaxMessageSize) { // Basic sanity check
                    logger().Log("Invalid message size received: " + std::to_string(msg_size));
                    break;
                }

                std::vector<char> buffer(msg_size);

                // Read the exact number of bytes for the message body
                received = recv(client_socket, buffer.data(), msg_size, MSG_WAITALL);
                if (received <= 0) {
                    break; // Client disconnected or error
                }

                if (received == static_cast<ssize_t>(msg_size)) {
                    handler(client_socket, std::string(buffer.begin(), buffer.end()));
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
 */
void Service::SendMessage(const std::string &message) const {
    if (sockfd_ < 0) {
        logger().Log("Not connected, cannot send message.");
        return;
    }
    // Send the size of the message first
    uint32_t msg_size = htonl(message.length()); // Use network byte order
    if (send(sockfd_, &msg_size, sizeof(msg_size), 0) < 0) {
        logger().Log("Failed to send message size: " + std::string(strerror(errno)));
        return;
    }

    // Then send the message itself
    if (send(sockfd_, message.c_str(), message.length(), 0) < 0) {
        logger().Log("Failed to send message data: " + std::string(strerror(errno)));
    }
}

/** 
 * @brief Sends a response back to a specific client.
 *
 * @param client_socket The socket file descriptor of the client.
 * @param message The message to send as a response.
 */
void Service::SendResponse(int client_socket, const std::string &message) const {
    // Send the size of the message first
    uint32_t msg_size = htonl(message.length());
    if (send(client_socket, &msg_size, sizeof(msg_size), 0) < 0) {
        logger().Log("Failed to send response size to client.");
        return;
    }

    // Then send the message itself
    if (send(client_socket, message.c_str(), message.length(), 0) < 0) {
        logger().Log("Failed to send response data to client.");
    }
}

/** 
 * @brief Receives a message from the connected server.
 *
 * @return The received message.
 */
std::optional<std::string> Service::ReceiveMessage() {
    if (sockfd_ < 0) {
        logger().Log("Socket is not connected.");
        return std::nullopt;
    }

    SetupSocketTimeout(sockfd_);

    uint32_t msg_size;
    // Read the 4-byte size prefix
    ssize_t received = recv(sockfd_, &msg_size, sizeof(msg_size), MSG_WAITALL);
    if (received <= 0) {
        if (received == 0) {
            logger().Log("Connection closed by server.");
        } else {
            logger().Log("Error receiving message size from server.");
        }
        return std::nullopt;
    }

    msg_size = ntohl(msg_size);
    if (msg_size == 0 || msg_size > kMaxMessageSize) { // Basic sanity check
        logger().Log("Invalid message size received: " + std::to_string(msg_size));
        return std::nullopt;
    }

    std::vector<char> buffer(msg_size);

    // Read the exact number of bytes for the message body
    received = recv(sockfd_, buffer.data(), msg_size, MSG_WAITALL);
    if (received <= 0) {
        if (received == 0) {
            logger().Log("Connection closed by server during message read.");
        } else {
            logger().Log("Error receiving message data from server.");
        }
        return std::nullopt;
    }

    return std::string(buffer.begin(), buffer.end());
}

/** 
 * @brief Sets up the socket with necessary options.
 *
 * Configures the socket to have a timeout for receiving and sending data.
 *
 * @param sockfd The socket file descriptor to configure.
 */
void Service::SetupSocketTimeout(int sockfd) const {
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    // Set the socket receive timeout
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        logger().Log("Failed to set socket receive timeout: " + std::string(strerror(errno)));
        return;
    }

    // Set the socket send timeout
    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
        logger().Log("Failed to set socket send timeout: " + std::string(strerror(errno)));
    }
}

/** 
 * @brief Creates a server socket and binds it to the specified path.
 *
 * @param socket_path The path to bind the server socket.
 * @return The file descriptor of the created server socket.
 */
int Service::CreateServerSocket(const std::string &socket_path) const {
    // Ensure the socket file does not already exist
    unlink(socket_path.c_str());

    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        logger().Log("Failed to create server socket: " + std::string(strerror(errno)));
        return -1;
    }

    struct sockaddr_un address;
    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, socket_path.c_str(), sizeof(address.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        logger().Log("Failed to bind server socket to " + socket_path + ": " + std::string(strerror(errno)));
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, 10) < 0) {
        logger().Log("Failed to listen on server socket: " + std::string(strerror(errno)));
        close(server_fd);
        return -1;
    }

    return server_fd;
}

} // namespace hello_ipc