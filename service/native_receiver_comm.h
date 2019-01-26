#ifndef NATIVE_RECEIVER_COMM_H
#define NATIVE_RECEIVER_COMM_H

#include <thread>

#include <boost/asio.hpp>

using boost_tcp = boost::asio::ip::tcp;

class NativeReceiverComm {
public:
    NativeReceiverComm() = default;
    ~NativeReceiverComm();

    int Run(short port);
    void Stop();

private:
    void _CommThreadRun();
    void _AsyncListen();
    void _OnNewChannel(boost::system::error_code ec, boost_tcp::socket sock);

private:
    boost::asio::io_context* m_ioCtx = nullptr;
    boost_tcp::acceptor* m_acceptor = nullptr;
    std::thread m_commThread;
};

#endif
