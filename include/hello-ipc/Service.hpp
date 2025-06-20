#ifndef HELLO_IPC_SERVICE_HPP_
#define HELLO_IPC_SERVICE_HPP_
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdexcept>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <thread>
#include <mutex>
#include <vector>
#include <utility>
#include <iostream>
#include "Logger.hpp"

/** * @file Service.hpp
 * @brief Base class for IPC services using TCP/IP sockets.
 *
 * This class provides methods for connecting to a server, sending and receiving messages,
 * and running as a broker service.
 */
class Service {
    public:
        Service(const std::string &serviceName);
        virtual ~Service();

        // --- Client Methods ---
        void connectToServer(const std::string &ip, int port);
        void sendMessage(const std::string &message) const;
        std::string receiveMessage();

        // --- Broker Method ---
        void runAsBroker(int port);

        // --- Utility Method ---
        static std::pair<std::string, std::string> parseKeyValue(const std::string &msg);

    protected:
        Logger logger_;
        int sockfd;
        std::string receive_buffer_;

    private:
        // --- Broker-specific members ---
        void handleBrokerClient(int client_socket);
        std::vector<int> subscribers_;
        std::mutex subscribers_mutex_;
};

#endif // HELLO_IPC_SERVICE_HPP_