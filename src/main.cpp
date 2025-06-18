#include <iostream>
#include <string>
#include "Service.hpp"
#include "LedManager.hpp"
#include "UpdateLed.hpp"
#include "Logger.hpp"


/** * @file main.cpp
 * @brief Main entry point for the IPC application.
 *
 * This file contains the main function that initializes the application,
 * determines whether to run as a server or client, and starts the appropriate service.
 */
int main(int argc, char* argv[]) {
    const std::string ip = "127.0.0.1";
    const int port = 12345;

    // Check if the first argument is "--server" to decide the role
    if (argc > 1 && std::string(argv[1]) == "--server") {
        // Run as the LedManager server by calling the static method
        std::cout << "Starting LedManager server..." << std::endl;
        Service::run_server(port);
    } else {
        // Run as the UpdateLed client
        std::cout << "Starting UpdateLed client..." << std::endl;
        try {
            // Create the client. This will connect to the server.
            UpdateLed updateLed(ip, port, argc, argv);

            // Call run() to start the argument processing and the interactive loop
            updateLed.run();

        } catch (const std::exception& e) {
            std::cerr << "An error occurred in the client: " << e.what() << std::endl;
            return 1;
        }
    }

    std::cout << "Application finished." << std::endl;
    return 0;
}