#include "UpdateLed.hpp"
#include "led_service.pb.h"

#include <string>
#include <algorithm>
#include <cctype>

namespace hello_ipc {

/** 
 * @brief Constructs an UpdateLed client that connects to the LedManager service.
 *
 * @param socket_path The path to the socket file for communication.
 * @param argc The argument count from the command line.
 * @param argv The argument vector from the command line.
 * @throws std::runtime_error if the socket connection fails.
 */
UpdateLed::UpdateLed(const std::string &socket_path, int argc, char** argv, bool connect)
    : Service("UpdateLed"), argc_(argc), argv_(argv) {
    if (connect) {
        ConnectToServer(socket_path);
    }
}

/** 
 * @brief Runs the UpdateLed client, processing command-line arguments and user input.
 *
 * This method handles command-line arguments to send initial LED updates,
 * and then enters a loop to handle user input for further updates.
 * 
 * @throws std::runtime_error if sending messages fails.
 */
void UpdateLed::Run() {
    if (argc_ > 2) { // The first arg is the program, second is the mode
        HandleArguments();
    }
    HandleUserInput(std::cin);
}

/** 
 * @brief Handles command-line arguments to send initial LED updates.
 *
 * This method processes arguments of the form "--led<number>" to send updates
 * for LEDs specified in the command line.
 * 
 * @throws std::runtime_error if sending messages fails.
 */
void UpdateLed::HandleArguments() {
    for (int i = 2; i < argc_; ++i) {
        std::string arg = argv_[i];
        if (arg.rfind("--led", 0) == 0) {
            std::string led_name = arg.substr(5);
            if (!led_name.empty()) {
                SendUpdate(led_name, "on");
            }
        }
    }
}

/**
 * @brief Handles user input for LED updates.
 *
 * This method reads user input from the console, allowing users to specify
 * LED numbers and their states (on/off) interactively.
 *
 * @param input_stream The input stream to read from (e.g., std::cin).
 */
void UpdateLed::HandleUserInput(std::istream &input_stream) {
    std::cout << "Welcome to the UpdateLed client!" << std::endl;
    std::cout << "Enter command ('1' for on, '!1' for off), or 'exit' to quit." << std::endl;
    std::string input;

    while (true) {
        if (&input_stream == &std::cin)
            std::cout << "> ";
        if (!std::getline(input_stream, input) || input == "exit")
            break;
        if (input.empty())
            continue;

        std::string led_name, led_state;
        if (input[0] == '!'){
            led_state = "off";
            led_name = input.substr(1);
        }  else {
            led_state = "on";
            led_name = input;
        }
        if (led_name.empty() || !std::all_of(led_name.begin(), led_name.end(), ::isdigit)) {
            std::cerr << "Invalid command." << std::endl;
            continue;
        }
        SendUpdate(led_name, led_state);
    }
}

/** 
 * @brief Sends an update for a specific LED to the LedManager service.
 *
 * Constructs a request message and sends it to the server, then waits for a response.
 *
 * @param led_name The name of the LED to update.
 * @param led_state The desired state of the LED ("on" or "off").
 * @throws std::runtime_error if sending or receiving messages fails.
 */
void UpdateLed::SendUpdate(const std::string &led_name, const std::string &led_state) {
    hello_ipc::Request req;
    auto* update_req = req.mutable_update_request();
    update_req->set_led_num(led_name);
    update_req->set_state((led_state == "on") ? hello_ipc::LedState::ON : hello_ipc::LedState::OFF);

    std::string message;
    if (!req.SerializeToString(&message)) {
        std::cerr << "Error: Failed to serialize request." << std::endl;
        return;
    }

    SendMessage(message);
    logger().Log("Sent update for LED " + led_name + " to state: " + led_state);

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
            std::cout << "Response: Led" << state_res.led_num() << " Updated" << std::endl;
        }
    }
    logger().Log("Received response: " + res.DebugString());
}

} // namespace hello_ipc