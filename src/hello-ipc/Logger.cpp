#include "Logger.hpp"
#include <stdexcept>

/**
 * @brief Constructs a Logger for the given service name.
 *        The log file will be /tmp/<service_name>.log
 */
Logger::Logger(const std::string &service_name)
    : service_name_(service_name) {
    OpenLogFile(service_name);
}

/**
 * @brief Opens the log file for the specified service name.
 *        The log file is created in /tmp/<service_name>.log format.
 * @param service_name The name of the service used to create the log file.
 * @throws std::runtime_error if the log file cannot be opened.
 */
void Logger::OpenLogFile(const std::string& service_name) {
    log_file_path_ = "/tmp/" + service_name + ".log";
    log_file_.open(log_file_path_, std::ios::app);
    if (!log_file_.is_open()) {
        throw std::runtime_error("Failed to open log file: " + log_file_path_);
    }
}

/**
 * @brief Logs a message to the log file.
 */
void Logger::Log(const std::string &message) const {
    if (log_file_.is_open()) {
        log_file_ << "[" << service_name_ << "]: " << message << std::endl;
    }
}

/**
 * @brief Destructor closes the log file.
 */
Logger::~Logger() {
    if (log_file_.is_open()) {
        log_file_.close();
    }
}