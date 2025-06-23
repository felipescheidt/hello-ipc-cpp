/** 
 * @file Service.cpp
 * @brief Implementation of the Service class for IPC using TCP/IP sockets.
 *
 * This file contains the implementation of methods for sending and receiving messages,
 * connecting to a server, and running a multi-threaded server loop.
 */

#include "Service.hpp"
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdexcept>
#include <cstring>
#include <iostream>
#include <thread>
#include <vector>

/** 
 * @brief Constructs a Service with the given service name.
 *
 * Initializes the logger and sets the socket file descriptor to -1 (not connected).
 *
 * @param serviceName The name of the service for logging purposes.
 */
Service::Service(const std::string &serviceName, bool connect)
            : logger_(serviceName), sockfd(-1) {
    (void)connect; // Suppress unused parameter warning
}


/** 
 * @brief Destructor closes the socket if it is open.
 */
Service::~Service() {
    if (sockfd >= 0) {
        close(sockfd);
    }
}

/** 
 * @brief Connects to a server at the specified socket path.
 *
 * @param socketPath The path to the socket file.
 * @throws std::runtime_error if the connection fails.
 */
void Service::connectToServer(const std::string &socketPath) {
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) throw std::runtime_error("Failed to create socket");

    struct sockaddr_un server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, socketPath.c_str(), sizeof(server_addr.sun_path) - 1);

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        throw std::runtime_error("Connection failed to " + socketPath);
    }
    logger_.log("Connection established to " + socketPath);
}

/** 
 * @brief Runs a multi-threaded server loop that listens for incoming connections.
 *
 * @param socketPath The path to the socket file.
 * @param messageHandler A function to handle incoming messages from clients.
 */
void Service::runServer(const std::string &socketPath,
                        const std::function<void(int, const std::string&)> &messageHandler) {
    // Ensure the socket file does not already exist
    unlink(socketPath.c_str());

    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket failed"); exit(EXIT_FAILURE);
    }

    struct sockaddr_un address;
    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, socketPath.c_str(), sizeof(address.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed"); exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen"); exit(EXIT_FAILURE);
    }

    logger_.log("Server listening on socket: " + socketPath);

    while (true) {
        int client_socket = accept(server_fd, NULL, NULL);
        if (client_socket < 0) {
            perror("accept"); continue;
        }
        logger_.log("Accepted new connection.");

        // Spawn a new thread to handle the client
        // The messageHandler is used to process incoming messages and send responses
        std::thread([this, client_socket, messageHandler]() {
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
                        messageHandler(client_socket, message);
                    }
                }
            }
            logger_.log("Client disconnected.");
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
void Service::sendMessage(const std::string &message) const {
    if (sockfd < 0) {
        throw std::runtime_error("Not connected");
    }
    if (send(sockfd, message.c_str(), message.length(), 0) < 0) {
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
void Service::sendResponse(int client_socket, const std::string &message) const {
    if (send(client_socket, message.c_str(), message.length(), 0) < 0) {
        logger_.log("Failed to send response to client.");
        throw std::runtime_error("Failed to send response");
    }
}

/** 
 * @brief Receives a message from the connected server.
 *
 * @return The received message.
 * @throws std::runtime_error if the connection is closed or receiving fails.
 */
std::string Service::receiveMessage() {
    char read_buffer[1024] = {0};

    while (receive_buffer_.find('\n') == std::string::npos) {
        ssize_t bytes_received = recv(sockfd, read_buffer, sizeof(read_buffer) - 1, 0);
        if (bytes_received <= 0) {
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
std::pair<std::string, std::string> Service::parseKeyValue(const std::string &msg) {
    size_t pos = msg.find('=');
    if (pos == std::string::npos) {
        return {msg, ""};
    }
    return {msg.substr(0, pos), msg.substr(pos + 1)};
}