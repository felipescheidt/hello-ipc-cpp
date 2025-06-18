/**
 * @file Service.cpp
 * @brief Implementation of the Service base class for IPC using TCP/IP sockets.
 */

#include "Service.hpp"
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "LedManager.hpp"

/**
 * @brief Constructs a Service object and establishes a TCP connection to the specified IP and port.
 * 
 * @param ip The IP address of the server.
 * @param port The port number of the server.
 * @throws std::runtime_error if socket creation, address conversion, or connection fails.
 */
Service::Service(const std::string &ip, int port, const std::string &serviceName, bool testMode)
    : logger_(serviceName), ip_(ip), port_(port) {

    if (!testMode) {
        setupSocket(ip, port);
    }
}

/**
 * @brief Sets up the socket connection to the specified IP and port.
 * 
 * This method creates a socket, sets up the server address structure, and connects to the server.
 * 
 * @param ip The IP address of the server.
 * @param port The port number of the server.
 * @throws std::runtime_error if socket creation, address conversion, or connection fails.
 */
void Service::setupSocket(const std::string &ip, int port) {
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

/**
 * @brief Destructor for the Service class.
 * 
 * Closes the socket if it is open.
 */
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

/**
 * @brief Runs the LedManager server on the specified port.
 * 
 * This method creates a TCP socket, binds it to the specified port, and listens for incoming connections.
 * It accepts client connections and processes messages from each client in a loop.
 * 
 * @param port The port number to listen on.
 */
void Service::run_server(int port) {
    LedManager ledManager;
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attach socket to the port, reusing it if necessary
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Bind the socket to the network address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    std::cout << "LedManager server listening on port " << port << std::endl;

    // Main outer loop to accept new client connections
    while (true) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
            perror("accept");
            continue; // On failure, wait for the next connection
        }

        std::cout << "New client connected." << std::endl;

        // Inner loop to handle all messages from the connected client
        while (true) {
            char buffer[1024] = {0};
            ssize_t bytes_read = read(new_socket, buffer, 1024);

            // If read() returns 0, the client has closed the connection.
            // If it returns < 0, an error occurred.
            if (bytes_read <= 0) {
                break; // Exit the inner loop to close this client's socket
            }

            std::string message(buffer, bytes_read);
            std::cout << "Server received: " << message << std::endl;
            try {
                ledManager.updateLedState(message);
            } catch (const std::exception& e) {
                std::cerr << "Error processing message: " << e.what() << std::endl;
            }
        }

        // The client has disconnected, so close their socket.
        std::cout << "Client disconnected. Closing socket." << std::endl;
        close(new_socket);
    }
}