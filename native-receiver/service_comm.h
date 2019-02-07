#ifndef SERVICE_COMM_H
#define SERVICE_COMM_H

#include <string>
#include <thread>
#include <future>

#include <boost/asio.hpp>

#include "comm_msg.h"

using boost_tcp = boost::asio::ip::tcp;

class ServiceComm {
public:
    ServiceComm();
    ~ServiceComm() = default;

    bool Connect();
    void Disconnect();

    bool Send(const std::string& data, std::string& rsp);

private:
    void ReadMsgHeader();
    void ReadMsgBody();
    void RunIoCtx();

private:
    std::thread m_ioCtxThread;
    boost_tcp::socket *m_sock = nullptr;
    boost::asio::io_context *m_ioCtx = nullptr;
    boost_tcp::resolver *m_resolver = nullptr;
    boost_tcp::resolver::results_type m_endpoint;
    std::promise<std::string> m_responsePromise;
    std::future<std::string> m_responseFuture;
    CommRecvMsg m_recvmsg;
    bool m_recvErr = true;
};

#endif
