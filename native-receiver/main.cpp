#include "logger.h"
#include "extension_comm.h"

int main(int argc, const char* argv[])
{
    int ret = 1;
    ExtensionComm extensionComm;
    std::string data;
    Logger::LoggerConfig config = { "native-receiver", false, false, true, true, true, true };

    logger.Init(config);

    LOG_INFO("native-receiver started");

    do {
        if (extensionComm.RecvFromExtension(data) == false) {
            LOG_ERROR("could not get the message");
            break;
        }

        ret = 0;
    } while (false);

    LOG_INFO("native-receiver exited with [%d]", ret);
    return ret;
}