/**
 * @file Service.cpp
 * @brief Implementation of the Service base class for IPC using TCP/IP sockets.
 */

#include "Service.hpp"
#include <cstring>
#include <arpa/inet.h>

/**
 * @brief Constructs a Service object and establishes a TCP connection to the specified IP and port.
 * 
 * @param ip The IP address of the server.
 * @param port The port number of the server.
 * @throws std::runtime_error if socket creation, address conversion, or connection fails.
 */
Service::Service(const std::string &ip, int port, const std::string &serviceName)
    : logger_(serviceName), ip_(ip), port_(port) {

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        throw std::runtime_error("Failed to create socket");
    }
    logger_.log("Started socket");

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr) <= 0) {
        close(sockfd);
        throw std::runtime_error("Invalid IP address");
    }

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(sockfd);
        throw std::runtime_error("Connection failed");
    }

    logger_.log("New connection established to " + ip + ":" + std::to_string(port));
}

Service::~Service() {
    if (sockfd >= 0) {
        close(sockfd);
    }
}

/**
 * @brief Sends a message to the IPC system.
 * 
 * @param message The message to send.
 * @throws std::runtime_error if sending the message fails.
 */
void Service::sendMessage(const std::string &message) const {
    ssize_t sent = send(sockfd, message.c_str(), message.size(), 0);

    if (sent < 0) {
        throw std::runtime_error("Failed to send message");
    }

    logger_.log("Sent message: " + message);
}

/** 
 * @brief Receives a message from the IPC system.
 * 
 * @return The received message as a string.
 * @throws std::runtime_error if receiving the message fails.
 */
std::string Service::receiveMessage() const {
    char buffer[1024] = {0};
    ssize_t received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);

    if (received < 0) {
        throw std::runtime_error("Failed to receive message");
    }

    std::string message(buffer, received);
    logger_.log("Received message: " + message);
    return message;
}

/**
 * @brief Parses a key-value pair from a message string.
 * 
 * The message is expected to be in the format "key=value". If no '=' is found,
 * the entire string is treated as the key with an empty value.
 * 
 * @param msg The message string to parse.
 * @return A pair containing the key and value.
 */
std::pair<std::string, std::string> Service::parseKeyValue(const std::string &msg) {
    size_t pos = msg.find('=');

    if (pos == std::string::npos) {
        return {msg, ""};
    }

    return {msg.substr(0, pos), msg.substr(pos + 1)};
}