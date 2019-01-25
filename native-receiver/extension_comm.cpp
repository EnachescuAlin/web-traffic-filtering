#include "extension_comm.h"
#include "logger.h"

#include <iostream>

struct DataSize {
    union {
        uint32_t size;
        uint8_t buffer[sizeof(size)];
    };
};

bool ExtensionComm::RecvFromExtension(std::string& data)
{
    data.clear();

    bool ret = false;
    DataSize dataSize = { 0 };
    int byte;
    size_t i;

    do {
        for (i = 0; i < sizeof(dataSize.size); i++) {
            byte = getchar();
            if (byte == EOF) {
                LOG_ERROR("received EOF while reading message size [current byte = %s]",
                    std::to_string(i).c_str());
                break;
            }

            dataSize.buffer[i] = static_cast<uint8_t>(byte);
        }
        if (i != sizeof(dataSize)) {
            break;
        }

        LOG_INFO("received a message of %s bytes", std::to_string(dataSize.size).c_str());

        for (i = 0; i < dataSize.size; i++) {
            byte = getchar();
            if (byte == EOF) {
                LOG_ERROR("received EOF while reading the message [current byte = %s]",
                    std::to_string(i).c_str());
                break;
            }

            data += static_cast<char>(byte);
        }
        if (i != dataSize.size) {
            break;
        }

        LOG_INFO("received the message [%s]", data.c_str());

        ret = true;
    } while (false);

    return ret;
}

void ExtensionComm::SendToExtension(const std::string& message)
{
    DataSize dataSize = { 0 };
    dataSize.size = message.length();

    for (size_t i = 0; i < sizeof(dataSize.size); i++) {
        std::cout << static_cast<char>(dataSize.buffer[i]);
    }

    std::cout << message;
    std::cout.flush();
}
