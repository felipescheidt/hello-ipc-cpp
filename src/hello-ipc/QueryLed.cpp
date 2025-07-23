#include "QueryLed.hpp"
#include "led_service.pb.h"

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
 * Constructs a request to query the LED state and sends it to the LedManager service.
 *
 * @param led_name The name of the LED to query.
 */
void QueryLed::queryState(const std::string &led_name) {
    hello_ipc::Request req;
    auto* query_req = req.mutable_query_request();
    query_req->set_led_num(led_name);

    std::string message;
    if (!req.SerializeToString(&message)) {
        std::cerr << "Error: Failed to serialize request." << std::endl;
        return;
    }

    SendMessage(message);
    logger().Log("Sent query for LED " + led_name);

    auto response_opt = ReceiveMessage();
    if (!response_opt) {
        std::cerr << "Error: Failed to receive response from server." << std::endl;
        return;
    }

    hello_ipc::Response res;
    if (!res.ParseFromString(*response_opt)) {
        std::cerr << "Error: Failed to parse response." << std::endl;
        return;
    }

    if (res.has_state_response()) {
        const auto& state_res = res.state_response();
        if (!state_res.error_message().empty()) {
            std::cout << "Error from server: " << state_res.error_message() << std::endl;
        } else {
            std::string state = (state_res.state() == hello_ipc::LedState::ON) ? "on" : "off";
            std::cout << "Response: Led" << state_res.led_num() << "=" << state << std::endl;
        }
    }
    logger().Log("Received response: " + res.DebugString());
}

} // namespace hello_ipc