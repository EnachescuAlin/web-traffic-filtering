#ifndef REQUESTS_H
#define REQUESTS_H

#include <map>
#include <string>
#include <mutex>
#include <memory>
#include <optional>

#include <nlohmann/json.hpp>

class Requests {
public:
    Requests() = default;
    ~Requests() = default;

    std::string OnMsg(const char *msg);

private:
    class Request {
    public:
        Request(uint64_t id);
        ~Request() = default;

        std::string OnMsg(const std::string& state, const nlohmann::json& msg, bool& removeReq);

    private:
        std::string FakeScanEngine(bool& removeReq);

        void OnMsgBeforeRequest(const nlohmann::json& msg);
        void OnMsgBeforeSendHeaders(const nlohmann::json& msg);
        void OnMsgHeadersReceived(const nlohmann::json& msg);
        void OnMsgResponseCompleted(const nlohmann::json& msg);

        std::string ScanEngineBeforeRequest(bool& removeReq);
        std::string ScanEngineBeforeSendHeaders(bool& removeReq);
        std::string ScanEngineHeadersReceived(bool& removeReq);
        std::string ScanEngineResponseCompleted(bool& removeReq);

    private:
        uint64_t m_reqId;
        std::mutex m_lock;
        std::string m_state;

        // request data
        std::optional<std::string> m_documentUrl;
        std::optional<std::string> m_originalUrl;
        std::optional<std::string> m_url;
        std::string m_method;
        std::string m_requestBody;

        // request headers
        std::map<std::string, std::string> m_requestHeaders;

        // response headers
        std::map<std::string, std::string> m_responseHeaders;
        std::string m_statusLine;

        // response body
        std::string m_responseBody;
    };

private:
    void InsertRequestPtr(uint64_t id, std::shared_ptr<Requests::Request> ptr);
    void RemoveRequestPtr(uint64_t id);
    std::shared_ptr<Request> GetRequestPtr(uint64_t id);

private:
    std::map<uint64_t, std::shared_ptr<Request>> m_requests;
};

#endif
