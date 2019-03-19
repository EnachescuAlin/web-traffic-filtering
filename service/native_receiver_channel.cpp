#include "native_receiver_channel.h"
#include "logger.h"

#include <nlohmann/json.hpp>

#include <atomic>

std::atomic<uint64_t> g_channelId(1);

#define CHANNEL_LOG_INFO(fmt, ...) LOG_INFO("[channel_%llu] " fmt, m_id, ##__VA_ARGS__)
#define CHANNEL_LOG_ERROR(fmt, ...) LOG_ERROR("[channel_%llu] " fmt, m_id, ##__VA_ARGS__)

NativeReceiverChannel::NativeReceiverChannel(boost_tcp::socket sock, Requests& requests)
    : m_sock(std::move(sock))
    , m_id(g_channelId.fetch_add(1))
    , m_requests(requests)
{
    CHANNEL_LOG_INFO("created channel");
}

NativeReceiverChannel::~NativeReceiverChannel()
{
    CHANNEL_LOG_INFO("destroyed channel");
}

void NativeReceiverChannel::Start()
{
    CHANNEL_LOG_INFO("started channel");
    ReadMsgHeader();
}

void NativeReceiverChannel::ReadMsgHeader()
{
    auto self(shared_from_this());
    boost::asio::async_read(
        m_sock,
        boost::asio::buffer(
            m_msg.GetHeaderBuffer(),
            m_msg.GetHeaderSize()
        ),
        [self](boost::system::error_code ec, std::size_t length) {
            uint64_t m_id = self->m_id;
            if (length != self->m_msg.GetHeaderSize()) {
                CHANNEL_LOG_ERROR("could not read msg header, length = %s", std::to_string(length).c_str());
                self->m_sock.close();
            } else {
                if (!ec) {
                    CHANNEL_LOG_INFO("received msg header, msg size = %s", std::to_string(self->m_msg.GetBodySize()).c_str());
                    self->ReadMsgBody();
                } else {
                    CHANNEL_LOG_ERROR("could not read msg header [%d]", ec.value());
                    self->m_sock.close();
                }
            }
        }
    );
}

void NativeReceiverChannel::ReadMsgBody()
{
    auto self(shared_from_this());
    boost::asio::async_read(
        m_sock,
        boost::asio::buffer(
            m_msg.GetBodyBuffer(),
            m_msg.GetBodySize()
        ),
        [self](boost::system::error_code ec, std::size_t length) {
            uint64_t m_id = self->m_id;
            if (length != self->m_msg.GetBodySize()) {
                CHANNEL_LOG_ERROR("could not read msg body, length = %s, body size = %s",
                    std::to_string(length).c_str(),
                    std::to_string(self->m_msg.GetBodySize()).c_str()
                );
                self->m_sock.close();
            } else {
                if (!ec) {
                    CHANNEL_LOG_INFO("received msg body");
                    self->ProcessingMsg();
                    self->m_sock.close();
                } else {
                    CHANNEL_LOG_ERROR("could not read msg body [%d]", ec.value());
                    self->m_sock.close();
                }
            }
        }
    );
}

void NativeReceiverChannel::ProcessingMsg()
{
    std::string rawMsg(m_msg.GetBuffer(), m_msg.GetBodySize());
    SendMsg(m_requests.OnMsg(rawMsg.c_str()));
}

void NativeReceiverChannel::SendMsg(const std::string& msg)
{
    CHANNEL_LOG_INFO("send msg [%s]", msg.c_str());

    CommSendMsg comm_msg(msg);
    boost::asio::write(m_sock, boost::asio::buffer(comm_msg.GetBuffer(), comm_msg.GetSize()));
}
