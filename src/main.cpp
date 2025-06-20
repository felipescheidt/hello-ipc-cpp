#include "UpdateLed.hpp"
#include "LedManager.hpp"
#include "Service.hpp" // Include Service to run the broker
#include <iostream>
#include <string>
#include <vector>

// Constants for the broker connection
const int BROKER_PORT = 8080;
const std::string BROKER_IP = "127.0.0.1";

/** * @brief Main entry point for the application.
 *
 * This function checks command-line arguments to determine if it should run as a broker or a LED manager.
 * If no arguments are provided, it defaults to running the UpdateLed client.
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line arguments.
 * @return Exit status code.
 */
int main(int argc, char* argv[]) {
    if (argc > 1) {
        std::string mode = argv[1];
        if (mode == "--broker") {
            // Create a Service instance and run it as a broker
            Service broker("Broker");
            broker.runAsBroker(BROKER_PORT);
            return 0;
        }
        if (mode == "--led-manager") {
            LedManager ledManager;
            ledManager.run(BROKER_IP, BROKER_PORT);
            return 0;
        }
    }

    // Default mode is the UpdateLed client
    try {
        UpdateLed client(BROKER_IP, BROKER_PORT, argc, argv);
        client.run();
    } catch (const std::runtime_error& e) {
        std::cerr << "Client Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}