#include "LedManager.hpp"
#include "led_service.pb.h"

#include <filesystem>
#include <fstream>
#include <iostream>

namespace hello_ipc {

/**
 * @brief Constructs a LedManager service.
 */
LedManager::LedManager() : Service("LedManager", true) {}

/**
 * @brief Runs the LedManager server, listening for incoming connections.
 * 
 * @param socket_path Path to the socket file for communication.
 */
void LedManager::Run(const std::string &socket_path) {
    RunServer(socket_path, [this](int client_socket, const std::string &msg) {
        this->HandleMessage(client_socket, msg);
    });
}

/** 
 * @brief Handles incoming messages from clients.
 * 
 * Parses the message and dispatches it to the appropriate handler based on the request type.
 * 
 * @param client_socket The socket descriptor of the connected client.
 * @param message The received message from the client.
 */
void LedManager::HandleMessage(int client_socket, const std::string &message) {
    hello_ipc::Request req;

    if (!req.ParseFromString(message)) {
        logger().Log("Failed to parse request from client.");
        return;
    }

    hello_ipc::Response res;

    switch (req.request_type_case()) {
        case hello_ipc::Request::kUpdateRequest:
            HandleUpdateRequest(req.update_request(), res.mutable_state_response());
            break;
        case hello_ipc::Request::kQueryRequest:
            HandleQueryRequest(req.query_request(), res.mutable_state_response());
            break;
        case hello_ipc::Request::REQUEST_TYPE_NOT_SET:
        default:
            logger().Log("Received request with no type set");
            return;
    }

    std::string response_str;
    if (res.SerializeToString(&response_str)) {
        SendResponse(client_socket, response_str);
    } else {
        logger().Log("Failed to serialize response");
    }
}

/** 
 * @brief Handles a LedUpdateRequest.
 * 
 * Updates the state of the specified LED and sends a response back to the client.
 * 
 * @param req The update request message.
 * @param res The state response message to populate.
 */
void LedManager::HandleUpdateRequest(const LedUpdateRequest &req, LedStateResponse *res) {
    const std::string &led_state_str = (req.state() == hello_ipc::LedState::ON) ? "on" : "off";

    logger().Log("Received update for LED: " + req.led_num() + " to state: " + led_state_str);

    UpdateLedState(req.led_num(), req.state());

    res->set_led_num(req.led_num());

    if (UpdateLedState(req.led_num(), req.state())) {
        res->set_state(req.state());
    } else {
        res->set_error_message("Failed to update LED state on the system.");
    }
}

/** 
 * @brief Handles a LedQueryRequest.
 * 
 * Queries the state of the specified LED and sends a response back to the client.
 * 
 * @param req The query request message.
 * @param res The state response message to populate.
 */
void LedManager::HandleQueryRequest(const LedQueryRequest& req, LedStateResponse* res) {
    logger().Log("Received query for LED: " + req.led_num());

    std::string state = GetLedState(req.led_num());

    res->set_led_num(req.led_num());

    if (state == "on") {
        res->set_state(hello_ipc::LedState::ON);
    } else if (state == "off") {
        res->set_state(hello_ipc::LedState::OFF);
    } else {
        res->set_error_message(state);
    }
}

/** 
 * @brief Updates the state of a specific LED.
 * 
 * Writes the desired state to the LED's brightness file.
 * 
 * @param led_num The number of the LED to update.
 * @param led_state The desired state of the LED (ON or OFF).
 * @return True if the update was successful, false otherwise.
 */
bool LedManager::UpdateLedState(const std::string &led_num, hello_ipc::LedState led_state) {
    if (led_num.empty()) {
        logger().Log("Invalid empty LED number.");
        return false;
    }

    if (led_state != hello_ipc::LedState::ON && led_state != hello_ipc::LedState::OFF) {
        logger().Log("Invalid LED state: " + std::to_string(static_cast<int>(led_state)));
        return false;
    }

    std::string ledDir = "/tmp/sys/class/led_" + led_num;
    std::string filePath = ledDir + "/brightness";

    std::error_code ec;
    std::filesystem::create_directories(ledDir, ec);
    if (ec) {
        logger().Log("Error creating directory " + ledDir + ": " + ec.message());
        return false;
    }

    std::ofstream ledFile(filePath);
    if (!ledFile) {
        logger().Log("Error opening file for writing: " + filePath);
        return false;
    }

    ledFile << (led_state == hello_ipc::LedState::ON ? "1" : "0") << '\n';
    if (!ledFile) {
        logger().Log("Error writing to file: " + filePath);
        return false;
    }

    std::string led_state_str = (led_state == hello_ipc::LedState::ON) ? "on" : "off";
    logger().Log("Updated LED " + led_num + " to state: " + led_state_str);

    return true;
}

/** 
 * @brief Retrieves the current state of a specific LED.
 * 
 * Reads the brightness file of the LED and returns its state as a string.
 * 
 * @param led_num The number of the LED to query.
 * @return A string representing the LED state ("on", "off", or an error message).
 */
std::string LedManager::GetLedState(const std::string &led_num) const {
    if (led_num.empty()) {
        return "error: LED number cannot be empty";
    }

    if (!std::filesystem::exists("/tmp/sys/class/led_" + led_num)) {
        return "error: LED not found";
    }

    std::string filePath = "/tmp/sys/class/led_" + led_num + "/brightness";
    std::ifstream ledFile(filePath);

    if (!ledFile.is_open())
        return "error";

    std::string state;
    std::getline(ledFile, state);

    return (state == "1") ? "on" : "off";
}

} // namespace hello_ipc