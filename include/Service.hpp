#ifndef HELLO_IPC_SERVICE_HPP_
#define HELLO_IPC_SERVICE_HPP_
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdexcept>
#include "Logger.hpp"

/**
 * @file Service.hpp
 * @brief Abstract base class for IPC services using TCP/IP sockets.
 */
class Service {
public:
    virtual ~Service();

    // Sends a message to the IPC system
    void sendMessage(const std::string &message) const;

    // Receives a message from the IPC system
    std::string receiveMessage() const;

    // Helper to parse key-value from received message
    static std::pair<std::string, std::string> parseKeyValue(const std::string &msg);

protected:
    Service(const std::string &ip, int port, const std::string &serviceName);

    int sockfd;
    struct sockaddr_in server_addr;

    Logger logger_; // Logger instance for logging messages

private:
    std::string ip_;
    int port_;
};

#endif // HELLO_IPC_SERVICE_HPP_