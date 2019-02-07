#include "native_receiver_comm.h"
#include "native_receiver_channel.h"
#include "logger.h"

NativeReceiverComm::~NativeReceiverComm()
{
    if (m_acceptor) {
        delete m_acceptor;
    }
    if (m_ioCtx) {
        delete m_ioCtx;
    }
}

int NativeReceiverComm::Run(short port)
{
    try {
        LOG_INFO("run on port %d", static_cast<int>(port));

        m_ioCtx = new boost::asio::io_context();
        LOG_INFO("created io_context");

        m_acceptor = new boost_tcp::acceptor(*m_ioCtx, boost_tcp::endpoint(boost_tcp::v4(), port));
        LOG_INFO("created acceptor");

        _AsyncListen();
        LOG_INFO("set async listener");

        m_commThread = std::move(std::thread([this]() -> void { _CommThreadRun(); }));
        LOG_INFO("created thread");
    } catch (std::exception& e) {
        LOG_ERROR("exception occurred [%s]", e.what());
        return 1;
    }

    return 0;
}

void NativeReceiverComm::Stop()
{
    m_ioCtx->stop();
    m_commThread.join();
}

void NativeReceiverComm::_CommThreadRun()
{
    LOG_INFO("server is running");
    m_ioCtx->run();
    LOG_INFO("server is stopping");
}

void NativeReceiverComm::_AsyncListen()
{
    m_acceptor->async_accept(
        std::bind(
            &NativeReceiverComm::_OnNewChannel,
            this,
            std::placeholders::_1,
            std::placeholders::_2
        )
    );
}

void NativeReceiverComm::_OnNewChannel(boost::system::error_code ec, boost_tcp::socket sock)
{
    if (!ec) {
        LOG_INFO("creating a new channel");
        auto channel = std::make_shared<NativeReceiverChannel>(std::move(sock));
        if (channel == nullptr) {
            LOG_ERROR("could not create a new channel");
        } else {
            channel->Start();
            LOG_INFO("created a new channel");
        }
    } else {
        LOG_ERROR("called with error [%d]", ec.value());
    }
    _AsyncListen();
}
