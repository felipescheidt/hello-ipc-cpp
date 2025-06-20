#ifndef HELLO_IPC_LED_MANAGER_HPP_
#define HELLO_IPC_LED_MANAGER_HPP_
#include "Service.hpp"

/** * @file LedManager.hpp
 * @brief A service that manages LED states by receiving commands from a broker.
 *
 * This class connects to a message broker and listens for LED state update commands.
 * It updates the LED state by writing to the appropriate file in the /tmp/sys/class/led_<led_num> directory.
 */
class LedManager : public Service {
    public:
        LedManager();
        void run(const std::string& ip, int port);

    private:
        void updateLedState(const std::string &message);
};

#endif // HELLO_IPC_LED_MANAGER_HPP_