#include "LedManager.hpp"
#include "UpdateLed.hpp"
#include "QueryLed.hpp"
#include <iostream>
#include <string>
#include <vector>

/**
 * @brief Path to the socket file for the LedManager service.
 * 
 */
const std::string LED_MANAGER_SOCKET = "/tmp/led_manager.sock";

void PrintUsage() {
    std::cerr << "Usage: hello_ipc <mode> [options]\n"
              << "Modes:\n"
              << "  --led-manager    Run the LedManager server.\n"
              << "  --update-led     Run the UpdateLed client.\n"
              << "  --query-led      Run the QueryLed client.\n";
}

/** 
 * @brief Main entry point for the hello-ipc application.
 *
 * This function initializes the application based on the provided mode.
 * It can run either the LedManager server, the UpdateLed client or the QueryLed client.
 *
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return Exit status code.
 */
int main(int argc, char* argv[]) {
    if (argc < 2) {
        PrintUsage();
        return 1;
    }

    std::string mode = argv[1];

    try {
        if (mode == "--led-manager") {
            hello_ipc::LedManager server;
            server.Run(LED_MANAGER_SOCKET);
        } else if (mode == "--update-led") {
            hello_ipc::UpdateLed client(LED_MANAGER_SOCKET, argc, argv);
            client.Run();
        } else if (mode == "--query-led") {
            hello_ipc::QueryLed client(LED_MANAGER_SOCKET);
            client.Run();
        } else {
            PrintUsage();
            return 1;
        }
    } catch (const std::runtime_error &e) {
        std::cerr << "Runtime Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}