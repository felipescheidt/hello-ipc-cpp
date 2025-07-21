#include "UpdateLed.hpp"

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
 * @brief Sends an update command for a specific LED.
 *
 * This method constructs a message in the format "led_name=led_state\n"
 * and sends it to the LedManager service.
 *
 * @param led_name The name of the LED to update.
 * @param led_state The new state of the LED ("on" or "off").
 * @throws std::runtime_error if sending the message fails.
 */
void UpdateLed::SendUpdate(const std::string &led_name, const std::string &led_state) {
    std::string message = led_name + "=" + led_state + "\n"; // ex: "1=on\n"
    SendMessage(message);
    logger().Log("Sent update for LED " + led_name + " to state: " + led_state);

    try {
        std::string response = ReceiveMessage();
        logger().Log("Received response: " + response);
        std::cout << "Response: " << response << std::endl;
    } catch (const std::runtime_error &e) {
        std::cout << "Error receiving response: " << e.what() << std::endl;
    }
}

} // namespace hello_ipc