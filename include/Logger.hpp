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
     * @param serviceName The name of the service (used for the log file name).
     */
    explicit Logger(const std::string &serviceName);

    /**
     * @brief Logs a message to the log file.
     * @param message The message to log.
     */
    void log(const std::string &message) const;

    /**
     * @brief Destructor closes the log file.
     */
    ~Logger();

private:
    void openLogFile(const std::string& serviceName);
    std::string logFilePath_;
    std::string serviceName_;
    mutable std::ofstream logFile_;
};

#endif // HELLO_IPC_LOGGER_HPP_