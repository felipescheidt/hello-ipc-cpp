#include "Service.hpp"

/** * @file Service.cpp
 * @brief Implementation of the Service class for IPC using TCP/IP sockets.
 *
 * This class provides methods for connecting to a server, sending and receiving messages,
 * and running as a broker service.
 */
Service::Service(const std::string &serviceName)
    : logger_(serviceName), sockfd(-1) {}


/**
 * @brief Destructor closes the socket if it is open.
 *
 * This method ensures that the socket is properly closed when the Service object is destroyed.
 */
Service::~Service() {
    if (sockfd >= 0) {
        close(sockfd);
    }
}

/**
 * @brief Connects to a server at the specified IP address and port.
 *
 * This method creates a socket, sets up the server address structure, and attempts to connect.
 * Throws an exception if the connection fails.
 *
 * @param ip The IP address of the server.
 * @param port The port number of the server.
 */
void Service::connectToServer(const std::string &ip, int port) {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) throw std::runtime_error("Failed to create socket");

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr) <= 0) {
        close(sockfd);
        throw std::runtime_error("Invalid IP address");
    }

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(sockfd);
        throw std::runtime_error("Connection failed to " + ip + ":" + std::to_string(port));
    }
}

/**
 * @brief Sends a message to the connected server.
 *
 * This method sends a string message over the socket. Throws an exception if the socket is not connected
 * or if sending fails.
 *
 * @param message The message to send.
 */
void Service::sendMessage(const std::string &message) const {
    if (sockfd < 0) throw std::runtime_error("Not connected, cannot send message.");
    if (send(sockfd, message.c_str(), message.length(), 0) < 0) {
        throw std::runtime_error("Failed to send message");
    }
}

/**
 * @brief Receives a message from the connected server.
 *
 * This method reads data from the socket until it encounters a newline character.
 * It returns the complete message received, excluding the newline.
 * Throws an exception if the connection is closed or if reading fails.
 *
 * @return The received message as a string.
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
 * @brief Parses a key-value pair from a message string.
 *
 * This method splits the input string at the first '=' character.
 * If no '=' is found, it returns the entire string as the key and an empty string as the value.
 *
 * @param msg The input message string to parse.
 * @return A pair containing the key and value.
 */
std::pair<std::string, std::string> Service::parseKeyValue(const std::string &msg) {
    size_t pos = msg.find('=');
    if (pos == std::string::npos) return {msg, ""};
    return {msg.substr(0, pos), msg.substr(pos + 1)};
}


/**
 * @brief Runs the service as a message broker, listening for client connections.
 *
 * This method sets up a TCP server that listens for incoming connections on the specified port.
 * It spawns a new thread for each client connection to handle messages from publishers and subscribers.
 *
 * @param port The port number to listen on for incoming connections.
 */
void Service::runAsBroker(int port) {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed"); exit(EXIT_FAILURE);
    }
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt"); exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed"); exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 10) < 0) {
        perror("listen"); exit(EXIT_FAILURE);
    }

    logger_.log("Message Broker listening on port " + std::to_string(port));

    while (true) {
        int client_socket;
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
            perror("accept"); continue;
        }
        logger_.log("Accepted new connection. Spawning handler thread.");
        std::thread(&Service::handleBrokerClient, this, client_socket).detach();
    }
}

/**
 * @brief Handles a client connection in the broker.
 *
 * This method reads the first message from the client to determine if it is a publisher or subscriber.
 * If it is a subscriber, it registers the socket and keeps it open. If it is a publisher,
 * it enters a loop to forward all messages to all registered subscribers.
 *
 * @param client_socket The socket descriptor for the connected client.
 */
void Service::handleBrokerClient(int client_socket) {
    char buffer[1024] = {0};
    // Do a single blocking read to get the first message, which determines the client type.
    ssize_t bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

    if (bytes_read <= 0) {
        close(client_socket);
        return;
    }

    std::string initial_data(buffer, bytes_read);

    // Check if the first message is a subscription request.
    if (initial_data.rfind("SUBSCRIBE\n", 0) == 0) {
        std::lock_guard<std::mutex> lock(subscribers_mutex_);
        subscribers_.push_back(client_socket);
        logger_.log("Client registered as a SUBSCRIBER.");
        // This thread's job is done. The socket will remain open.
        // A more robust implementation would have a way to detect subscriber disconnects.
        return;
    }

    // If not a subscriber, it's a publisher. Loop and forward all its messages.
    logger_.log("Client is a PUBLISHER. Entering message forwarding loop.");
    std::string message_buffer = initial_data;

    while (true) {
        size_t pos;
        // Process all complete, newline-terminated messages currently in our buffer.
        while ((pos = message_buffer.find('\n')) != std::string::npos) {
            std::string message_to_forward = message_buffer.substr(0, pos + 1);
            message_buffer.erase(0, pos + 1);

            if (!message_to_forward.empty()) {
                std::lock_guard<std::mutex> lock(subscribers_mutex_);
                for (int sub_socket : subscribers_) {
                    send(sub_socket, message_to_forward.c_str(), message_to_forward.length(), 0);
                }
            }
        }

        // Try to read more data from the publisher client.
        memset(buffer, 0, sizeof(buffer));
        bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_read <= 0) {
            // The client has disconnected (e.g., typed 'exit').
            logger_.log("Publisher client disconnected.");
            break; // Exit the loop.
        }
        message_buffer.append(buffer, bytes_read);
    }

    // The loop is broken, so close the connection.
    close(client_socket);
}