/**
 * @file QueryLed.cpp
 * @brief Implementation of the QueryLed client for querying LED states via IPC.
 * This file contains methods for handling user input and sending queries to the LedManager service.
 */


#include "QueryLed.hpp"
#include <string>
#include <algorithm>
#include <cctype>

/** 
 * @brief Constructs a QueryLed client that connects to the LedManager service.
 *
 * @param socketPath The path to the socket file for communication.
 * @param connect Whether to connect to the server immediately.
 * @throws std::runtime_error if the socket connection fails.
 */
QueryLed::QueryLed(const std::string &socketPath, bool connect)
        : Service("QueryLed", connect) {
    if (connect) {
        connectToServer(socketPath);
    }
}

/** 
 * @brief Runs the QueryLed client, handling user input to query LED states.
 *
 * This method enters a loop to read user input and send queries to the LedManager service.
 * 
 * @throws std::runtime_error if sending messages fails.
 */
void QueryLed::run() {
    handleUserInput(std::cin);
}

/** 
 * @brief Handles user input for querying LED states.
 *
 * This method reads user input from the console, allowing users to specify
 * LED numbers to query interactively.
 *
 * @param inputStream The input stream to read from (e.g., std::cin).
 */
void QueryLed::handleUserInput(std::istream &inputStream) {
    std::cout << "Welcome to the QueryLed client!" << std::endl;
    std::cout << "Enter LED number to query (e.g., '1'), or 'exit' to quit." << std::endl;

    std::string input;
    while (true) {
        if (&inputStream == &std::cin) {
            std::cout << "> ";
        }
        if (!std::getline(inputStream, input) || input == "exit") {
            break;
        }
        if (input.empty()) continue;

        if (!std::all_of(input.begin(), input.end(), ::isdigit)) {
            std::cerr << "Invalid input. LED name must be a number." << std::endl;
            continue;
        }
        queryState(input);
    }
}

/** 
 * @brief Queries the state of a specific LED by sending a message to the LedManager service.
 *
 * This method constructs a query message in the format "QUERY=ledName\n"
 * and sends it to the LedManager service, then waits for a response.
 *
 * @param ledName The name of the LED to query.
 * @throws std::runtime_error if sending the message or receiving the response fails.
 */
void QueryLed::queryState(const std::string &ledName) {
    std::string message = "QUERY=" + ledName + "\n";
    sendMessage(message);
    logger_.log("Sent query for LED " + ledName);

    try {
        std::string response = receiveMessage();
        logger_.log("Received response: " + response);
        std::cout << "Response: " << response << std::endl;
    } catch (const std::runtime_error &e) {
        std::cout << "Error receiving response: " << e.what() << std::endl;
    }
}