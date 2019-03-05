#ifndef REQUESTS_H
#define REQUESTS_H

#include <map>
#include <iostream>

class Requests {
public:
    Requests() = default;
    ~Requests() = default;

    std::string OnMsg(const char *msg)
    {
        return "test";
    }

private:
    class Request {
    public:
        Request() = default;
        ~Request() = default;
    };

private:
    std::map<size_t, Request> m_requests;
};

#endif
