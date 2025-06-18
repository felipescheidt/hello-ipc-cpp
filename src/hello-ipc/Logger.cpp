#include "Logger.hpp"
#include <stdexcept>

/**
 * @brief Constructs a Logger for the given service name.
 *        The log file will be /tmp/<serviceName>.log
 */
Logger::Logger(const std::string &serviceName)
    : serviceName_(serviceName) {
    openLogFile(serviceName);
}

/**
 * @brief Opens the log file for the specified service name.
 *        The log file is created in /tmp/<serviceName>.log format.
 * @param serviceName The name of the service used to create the log file.
 * @throws std::runtime_error if the log file cannot be opened.
 */
void Logger::openLogFile(const std::string& serviceName) {
    logFilePath_ = "/tmp/" + serviceName + ".log";
    logFile_.open(logFilePath_, std::ios::app);
    if (!logFile_.is_open()) {
        throw std::runtime_error("Failed to open log file: " + logFilePath_);
    }
}

/**
 * @brief Logs a message to the log file.
 */
void Logger::log(const std::string &message) const {
    if (logFile_.is_open()) {
        logFile_ << "[" << serviceName_ << "]: " << message << std::endl;
    }
}

/**
 * @brief Destructor closes the log file.
 */
Logger::~Logger() {
    if (logFile_.is_open()) {
        logFile_.close();
    }
}