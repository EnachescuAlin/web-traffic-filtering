#include <iostream>
#include <string>

#include "logger.h"

int main(int argc, const char* argv[])
{
    int ret = 1;
    std::string command;
    Logger::LoggerConfig config = { "service", true, true, false, true, false, true };

    logger.Init(config);
    LOG_INFO("service started");

    do {
        std::cout << "type 'q' to exit" << std::endl;

        while (true) {
            std::getline(std::cin, command);
            if (command == "q") {
                std::cout << "exiting" << std::endl;
                break;
            } else {
                std::cout << "unknown command [" << command << "]" << std::endl;
            }
        }

        ret = 0;
    } while (false);

    LOG_INFO("service exited with [%d]", ret);
    return ret;
}
