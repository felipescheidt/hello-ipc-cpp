#ifndef HELLO_IPC_LOGGER_HPP_
#define HELLO_IPC_LOGGER_HPP_
#include <iostream>
#include <string>
#include <fstream>

/**
 * @file Logger.hpp
 * @brief Logger class for logging messages to file.
 *
 * This class provides methods to log messages to a specified log file.
 * It writes in the /tmp/<log-name>.log format and <log-name> is the name of the service ex: UpdateLed
 */
class Logger {
    public:
        /**
         * @brief Constructs a Logger for the given service name.
         * @param service_name The name of the service (used for the log file name).
         */
        explicit Logger(const std::string &service_name);

        /**
         * @brief Logs a message to the log file.
         * @param message The message to log.
         */
        void Log(const std::string &message) const;

        /**
         * @brief Destructor closes the log file.
         */
        ~Logger();

    private:
        void OpenLogFile(const std::string &service_name);
        std::string log_file_path_;
        std::string service_name_;
        mutable std::ofstream log_file_;
};

#endif // HELLO_IPC_LOGGER_HPP_