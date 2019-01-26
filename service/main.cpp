#include <iostream>
#include <string>

#include "logger.h"
#include "native_receiver_comm.h"

static int getPort(int argc, const char* argv[], short& port)
{
    if (argc < 2) {
        LOG_ERROR("argc [%d] < 2", argc);
        return 1;
    }

    port = static_cast<short>(std::atoi(argv[1]));
    LOG_INFO("detected port = [%d]", static_cast<int>(port));

    return 0;
}

int main(int argc, const char* argv[])
{
    int ret = 0;
    std::string command;
    NativeReceiverComm nativeReceiverComm;
    short port = 0;
    Logger::LoggerConfig config = { "service", true, true, false, true, false, true };

    logger.Init(config);
    LOG_INFO("service started");

    do {
        ret = getPort(argc, argv, port);
        if (ret != 0) {
            LOG_ERROR("could not get the port");
            break;
        }

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
