#include "QueryLed.hpp"

#include <string>
#include <algorithm>
#include <cctype>

namespace hello_ipc {

/**
 * @brief Constructs a QueryLed client.
 * 
 * Initializes the client and optionally connects to the LedManager service.
 *
 * @param socket_path Path to the socket file for communication.
 * @param connect Whether to connect to the server immediately.
 * @throws std::runtime_error If the socket connection fails.
 */
QueryLed::QueryLed(const std::string &socket_path, bool connect)
        : Service("QueryLed", connect) {
    if (connect) {
        ConnectToServer(socket_path);
    }
}

/**
 * @brief Runs the QueryLed client.
 * 
 * Handles user input to query LED states interactively.
 */
void QueryLed::Run() {
    HandleUserInput(std::cin);
}

/**
 * @brief Handles user input for querying LED states.
 * 
 * Reads user input from the console and sends queries to the LedManager service.
 *
 * @param input_stream Input stream to read from (e.g., std::cin).
 */
void QueryLed::HandleUserInput(std::istream &input_stream) {
    std::cout << "Welcome to the QueryLed client!" << std::endl;
    std::cout << "Enter LED number to query (e.g., '1'), or 'exit' to quit." << std::endl;

    std::string input;
    while (true) {
        if (&input_stream == &std::cin) {
            std::cout << "> ";
        }
        if (!std::getline(input_stream, input) || input == "exit") {
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
 * @brief Queries the state of a specific LED.
 * 
 * Sends a query message to the LedManager service and waits for a response.
 *
 * @param led_name Name of the LED to query.
 * @throws std::runtime_error If sending or receiving the message fails.
 */
void QueryLed::queryState(const std::string &led_name) {
    std::string message = "QUERY=" + led_name + "\n";
    SendMessage(message);
    logger().Log("Sent query for LED " + led_name);

    try {
        std::string response = ReceiveMessage();
        logger().Log("Received response: " + response);
        std::cout << "Response: " << response << std::endl;
    } catch (const std::runtime_error &e) {
        std::cout << "Error receiving response: " << e.what() << std::endl;
    }
}

} // namespace hello_ipc