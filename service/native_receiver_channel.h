#ifndef NATIVE_RECEIVER_CHANNEL_H
#define NATIVE_RECEIVER_CHANNEL_H

#include <memory>

#include <boost/asio.hpp>

#include "comm_msg.h"
#include "requests.h"

using boost_tcp = boost::asio::ip::tcp;

class NativeReceiverChannel : public std::enable_shared_from_this<NativeReceiverChannel> {
public:
    NativeReceiverChannel(boost_tcp::socket sock, Requests& requests);
    ~NativeReceiverChannel();

    void Start();

private:
    void ReadMsgHeader();
    void ReadMsgBody();
    void SendMsg(const std::string& msg);
    void ProcessingMsg();

private:
    CommRecvMsg m_msg;
    boost_tcp::socket m_sock;
    uint64_t m_id;
    Requests& m_requests;
};

#endif
