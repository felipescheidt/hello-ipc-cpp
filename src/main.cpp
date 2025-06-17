#include <iostream>
#include <string>
#include "Service.hpp"
#include "LedManager.hpp"
#include "UpdateLed.hpp"
#include "Logger.hpp"

/**
 * @file main.cpp
 * @brief Main entry point for the application.
 *
 * This file contains the main function that initializes the application and performs basic operations.
 */
int main(int argc, char* argv[]) {
    std::cout << "Starting the application..." << std::endl;

    if (argc) {
        std::cout << argc << " arguments provided." << std::endl;
        for (int i = 0; i < argc; ++i) {
            std::cout << "Argument " << i << ": " << argv[i] << std::endl;
        }
    }

    return 0;
}