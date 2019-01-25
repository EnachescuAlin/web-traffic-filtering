#ifndef EXTENSION_COMM_H
#define EXTENSION_COMM_H

#include <string>

class ExtensionComm {
public:
    ExtensionComm() = default;
    ~ExtensionComm() = default;

    /*
     * returns true if a message was received or false if a error occurred
     */
    bool RecvFromExtension(std::string& data);

    void SendToExtension(const std::string& message);
};

#endif
