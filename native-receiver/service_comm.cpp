#include "service_comm.h"
#include "logger.h"
#include "comm.h"

ServiceComm::ServiceComm()
   // : m_responseFuture(m_responsePromise.get_future())
{
    m_responseFuture = m_responsePromise.get_future();
}

bool ServiceComm::Connect()
{
    try {
        m_ioCtx = new boost::asio::io_context();
        LOG_INFO("created io_context");

        m_resolver = new boost_tcp::resolver(*m_ioCtx);
        LOG_INFO("created resolver");

        m_sock = new boost_tcp::socket(*m_ioCtx);
        LOG_INFO("created socket");

        m_endpoint = m_resolver->resolve("localhost", std::to_string(COMM_PORT));

        std::promise<bool> connectResultPromise;
        std::future<bool> connectResultFuture = connectResultPromise.get_future();

        boost::asio::async_connect(
            *m_sock,
            m_endpoint,
            [this, &connectResultPromise](boost::system::error_code ec, boost_tcp::endpoint) {
                if (!ec) {
                    LOG_INFO("connected successfully to service");
                    connectResultPromise.set_value(true);
                    ReadMsgHeader();
                } else {
                    LOG_ERROR("could not connect to service [%d]", ec.value());
                    connectResultPromise.set_value(false);
                }
            }
        );

        m_ioCtxThread = std::move(std::thread([this]() -> void { RunIoCtx(); }));
        LOG_INFO("created thread");

        LOG_INFO("waiting to connect");
        connectResultFuture.wait();

        bool connectRes = connectResultFuture.get();
        LOG_INFO("connect returned [%s]", connectRes ? "true" : "false");

        if (connectRes == false) {
            Disconnect();
            return false;
        }
    } catch (std::exception& e) {
        LOG_ERROR("exception occurred [%s]", e.what());
        Disconnect();
        return false;
    }

    return true;
}

void ServiceComm::Disconnect()
{
    if (m_ioCtxThread.joinable()) {
        m_ioCtx->stop();
        m_ioCtxThread.join();
    }

    if (m_resolver != nullptr) {
        delete m_resolver;
        m_resolver = nullptr;
    }

    if (m_sock != nullptr) {
        delete m_sock;
        m_sock = nullptr;
    }

    if (m_ioCtx != nullptr) {
        delete m_ioCtx;
        m_ioCtx = nullptr;
    }
}

bool ServiceComm::Send(const std::string& data, std::string& rsp)
{
    LOG_INFO("send to service [%s]", data.c_str());

    rsp.clear();
    CommSendMsg sendmsg(data);

    try {
        auto ret = boost::asio::write(
            *m_sock,
            boost::asio::buffer(
                sendmsg.GetBuffer(),
                sendmsg.GetSize()
            )
        );
        if (ret != sendmsg.GetSize()) {
            LOG_ERROR("write returned [%s], expected [%d]",
                std::to_string(ret).c_str(),
                std::to_string(sendmsg.GetSize()).c_str()
            );
            m_sock->close();
            return false;
        }

        LOG_INFO("waiting for response");
        m_responseFuture.wait();

        if (m_recvErr == true) {
            LOG_ERROR("recv error is set");
            // don't call sock->close() because it already was called by recv thread
            return false;
        }

        rsp = m_responseFuture.get();
    } catch (std::exception& e) {
        LOG_ERROR("exception occurred [%s]", e.what());
        m_sock->close();
        return false;
    }

    return true;
}

void ServiceComm::RunIoCtx()
{
    LOG_INFO("running io ctx");
    m_ioCtx->run();
    LOG_INFO("stopping io ctx");
}

void ServiceComm::ReadMsgHeader()
{
    boost::asio::async_read(
        *m_sock,
        boost::asio::buffer(
            m_recvmsg.GetHeaderBuffer(),
            m_recvmsg.GetHeaderSize()
        ),
        [this](boost::system::error_code ec, std::size_t length) {
            if (length != m_recvmsg.GetHeaderSize()) {
                LOG_ERROR("could not read msg header, length = %s", std::to_string(length).c_str());
                m_sock->close();
                m_responsePromise.set_value(std::string());
            } else {
                if (!ec) {
                    LOG_INFO("received msg header");
                    ReadMsgBody();
                } else {
                    LOG_ERROR("could not read msg header [%d]", ec.value());
                    m_sock->close();
                    m_responsePromise.set_value(std::string());
                }
            }
        }
    );
}

void ServiceComm::ReadMsgBody()
{
    boost::asio::async_read(
        *m_sock,
        boost::asio::buffer(
            m_recvmsg.GetBodyBuffer(),
            m_recvmsg.GetBodySize()
        ),
        [this](boost::system::error_code ec, std::size_t length) {
            if (length != m_recvmsg.GetBodySize()) {
                LOG_ERROR("could not read msg body, length = %s, body size = %s",
                    std::to_string(length).c_str(),
                    std::to_string(m_recvmsg.GetBodySize()).c_str()
                );
                m_sock->close();
                m_responsePromise.set_value(std::string());
            } else {
                if (!ec) {
                    LOG_INFO("received msg body");
                    m_recvErr = false;
                    m_responsePromise.set_value(std::string(m_recvmsg.GetBodyBuffer(), m_recvmsg.GetBodySize()));
                    m_sock->close();
                } else {
                    LOG_ERROR("could not read msg body [%d]", ec.value());
                    m_sock->close();
                    m_responsePromise.set_value(std::string());
                }
            }
        }
    );
}
