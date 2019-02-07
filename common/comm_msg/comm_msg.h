#ifndef COMM_MSG_H
#define COMM_MSG_H

#include <string>
#include <cstring>
#include <cstdint>

class CommSendMsg {
public:
    CommSendMsg(const std::string& data)
    {
        uint32_t bodySize = data.size();
        size = 4 + bodySize;
        buffer = new (std::nothrow) char[size];
        if (buffer != nullptr) {
            memcpy(buffer, &bodySize, 4);
            memcpy(buffer + 4, data.c_str(), bodySize);
        } else {
            size = 0;
        }
    }

    ~CommSendMsg()
    {
        if (buffer != nullptr) {
            delete[] buffer;
            buffer = nullptr;
        }
    }

    bool IsValid() const
    {
        return buffer != nullptr;
    }

    const char* GetBuffer() const
    {
        return buffer;
    }

    uint32_t GetSize() const
    {
        return size;
    }

private:
    char *buffer = nullptr;
    uint32_t size = 0;
};

class CommRecvMsg {
public:
    CommRecvMsg()
    {
    }

    ~CommRecvMsg()
    {
        if (buffer != nullptr) {
            delete[] buffer;
            buffer = nullptr;
        }
    }

    char* GetHeaderBuffer()
    {
        return reinterpret_cast<char*>(&size);
    }

    uint32_t GetHeaderSize()
    {
        return 4;
    }

    char* GetBodyBuffer()
    {
        if (buffer == nullptr) {
            buffer = new (std::nothrow) char[size];
        }
        return buffer;
    }

    uint32_t GetBodySize()
    {
        return size;
    }

    const char* GetBuffer() const
    {
        return buffer;
    }

    uint32_t GetSize() const
    {
        return size;
    }

private:
    uint32_t size = 0;
    char *buffer = nullptr;
};

#endif
