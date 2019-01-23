#include <iostream>

#include <unistd.h>
#include "logger.h"

int main(int argc, const char* argv[])
{
    int ret = 0;
    Logger::LoggerConfig config = { "native-receiver", false, false, true, true, true, true };
    logger.Init(config);

    LOG_INFO("native-receiver started");

    do {
        // todo
    } while (false);

    LOG_INFO("native-receiver exited with [%d]", ret);
    return ret;
}