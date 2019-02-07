#include <iostream>
#include <string>

#include "logger.h"
#include "native_receiver_comm.h"
#include "comm.h"

int main(int argc, const char* argv[])
{
    int ret = 0;
    std::string command;
    NativeReceiverComm nativeReceiverComm;
    short port = COMM_PORT;
    Logger::LoggerConfig config = { "service", true, true, false, true, false, true };

    logger.Init(config);
    LOG_INFO("service started");

    do {
        LOG_INFO("port = %d", static_cast<int>(port));

        ret = nativeReceiverComm.Run(port);
        if (ret != 0) {
            LOG_ERROR("could not run the server");
            break;
        }

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

        nativeReceiverComm.Stop();
    } while (false);

    LOG_INFO("service exited with [%d]", ret);
    return ret;
}
