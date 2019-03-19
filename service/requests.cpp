#include "requests.h"
#include "logger.h"

#define REQUEST_LOG_INFO(fmt, ...) LOG_INFO("[request_%llu] " fmt, m_reqId, ##__VA_ARGS__)
#define REQUEST_LOG_ERROR(fmt, ...) LOG_ERROR("[request_%llu] " fmt, m_reqId, ##__VA_ARGS__)

std::mutex requestsLock;

static std::string generate_error(const char *msg)
{
    nlohmann::json error;
    error["error"] = msg;
    return error.dump();
}

std::string Requests::OnMsg(const char *msg)
{
    nlohmann::json jsonMsg;
    try {
        jsonMsg = nlohmann::json::parse(msg);
    } catch (nlohmann::json::parse_error&) {
        LOG_ERROR("could not parse message from native_receiver [%s]", msg);
        return generate_error("could not parse response from native-receiver");
    }

    uint64_t reqId = std::stoull(jsonMsg["requestId"].get<std::string>());
    std::string state = jsonMsg["state"].get<std::string>();
    std::shared_ptr<Request> reqPtr;
    bool removeReq = false;

    if (state == "onBeforeRequest") {
        LOG_INFO("created a new request, id = %llu", reqId);

        reqPtr = std::make_shared<Request>(reqId);
        InsertRequestPtr(reqId, reqPtr);
    } else {
        LOG_INFO("processing msg, state = [%s], id = %llu", state.c_str(), reqId);

        reqPtr = GetRequestPtr(reqId);
        if (reqPtr == nullptr) {
            LOG_ERROR("not found reqPtr");
            return generate_error("not found request");
        }
    }

    auto rsp = reqPtr->OnMsg(state, jsonMsg, removeReq);
    if (removeReq == true) {
        RemoveRequestPtr(reqId);
    }

    return rsp;
}

void Requests::InsertRequestPtr(uint64_t id, std::shared_ptr<Requests::Request> ptr)
{
    std::lock_guard<std::mutex> lock(requestsLock);
    m_requests[id] = ptr;
}

void Requests::RemoveRequestPtr(uint64_t id)
{
    std::lock_guard<std::mutex> lock(requestsLock);
    m_requests.erase(id);
}

std::shared_ptr<Requests::Request> Requests::GetRequestPtr(uint64_t id)
{
    std::lock_guard<std::mutex> lock(requestsLock);

    auto it = m_requests.find(id);
    if (it == m_requests.end()) {
        return nullptr;
    }

    return it->second;
}

Requests::Request::Request(uint64_t id)
    : m_reqId(id)
{
}

std::string Requests::Request::OnMsg(const std::string& state, const nlohmann::json& msg, bool& removeReq)
{
    removeReq = false;

    std::lock_guard<std::mutex> lock(m_lock);

    m_state = state;

    if (state == "onBeforeRequest") {
        OnMsgBeforeRequest(msg);
    } else if (state == "onBeforeSendHeaders") {
        OnMsgBeforeSendHeaders(msg);
    } else if (state == "onHeadersReceived") {
        OnMsgHeadersReceived(msg);
    } else if (state == "onResponseCompleted") {
        OnMsgResponseCompleted(msg);
    } else {
        REQUEST_LOG_ERROR("unknown state [%s]", state.c_str());
        return generate_error("unknown state");
    }

    return FakeScanEngine(removeReq);
}

void Requests::Request::OnMsgBeforeRequest(const nlohmann::json& msg)
{
    if (msg.find("documentUrl") != msg.end()) {
        m_documentUrl = msg["documentUrl"];
        REQUEST_LOG_INFO("document url = [%s]", m_documentUrl->c_str());
    }
    if (msg.find("originalUrl") != msg.end()) {
        m_originalUrl = msg["originalUrl"];
        REQUEST_LOG_INFO("document url = [%s]", m_originalUrl->c_str());
    }
    if (msg.find("url") != msg.end()) {
        m_url = msg["url"];
        REQUEST_LOG_INFO("url = [%s]", m_url->c_str());
    }
    if (msg.find("method") != msg.end()) {
        m_method = msg["method"];
        REQUEST_LOG_INFO("method = [%s]", m_method.c_str());
    }
    if (msg.find("requestBody") != msg.end()) {
        m_requestBody = msg["requestBody"];
        REQUEST_LOG_INFO("requestBody = [%s]", m_requestBody.c_str());
    }
}

void Requests::Request::OnMsgBeforeSendHeaders(const nlohmann::json& msg)
{
    if (msg.find("requestHeaders") != msg.end()) {
        for (auto it = msg["requestHeaders"].begin(); it != msg["requestHeaders"].end(); ++it) {
            auto name = it->find("name");
            auto value = it->find("value");
            if (name != it->end() && value != it->end()) {
                auto nameStr = name->get<std::string>();
                auto valueStr = value->get<std::string>();
                m_requestHeaders[nameStr] = valueStr;
                REQUEST_LOG_INFO("request header [%s] = [%s]", nameStr.c_str(), valueStr.c_str());
            }
        }
    }
}

void Requests::Request::OnMsgHeadersReceived(const nlohmann::json& msg)
{
    if (msg.find("responseHeaders") != msg.end()) {
        for (auto it = msg["responseHeaders"].begin(); it != msg["responseHeaders"].end(); ++it) {
            auto name = it->find("name");
            auto value = it->find("value");
            if (name != it->end() && value != it->end()) {
                auto nameStr = name->get<std::string>();
                auto valueStr = value->get<std::string>();
                m_responseHeaders[nameStr] = valueStr;
                REQUEST_LOG_INFO("request header [%s] = [%s]", nameStr.c_str(), valueStr.c_str());
            }
        }
    }
    if (msg.find("statusLine") != msg.end()) {
        m_statusLine = msg["statusLine"];
        REQUEST_LOG_INFO("statusLine = [%s]", m_statusLine.c_str());
    }
}

void Requests::Request::OnMsgResponseCompleted(const nlohmann::json& msg)
{
    if (msg.find("data") != msg.end()) {
        m_responseBody = msg["data"];
        REQUEST_LOG_INFO("data = [%s]", m_responseBody.c_str());
    }
}

std::string Requests::Request::FakeScanEngine(bool& removeReq)
{
    if (m_state == "onBeforeRequest") {
        return ScanEngineBeforeRequest(removeReq);
    }
    if (m_state == "onBeforeSendHeaders") {
        return ScanEngineBeforeSendHeaders(removeReq);
    }
    if (m_state == "onHeadersReceived") {
        return ScanEngineHeadersReceived(removeReq);
    }
    if (m_state == "onResponseCompleted") {
        return ScanEngineResponseCompleted(removeReq);
    }

    return generate_error("scan engine error - unknown state");
}

std::string Requests::Request::ScanEngineBeforeRequest(bool& removeReq)
{
    nlohmann::json rsp;
    rsp["status"] = "filter";

    if (m_url.has_value()) {
        if (*m_url == "http://www.phiorg.ro/web_traffic_filtering/cancel_request") {
            rsp["status"] = "cancel";
            removeReq = true;
        } else if (*m_url == "http://www.phiorg.ro/web_traffic_filtering/redirect_request") {
            rsp["status"] = "redirect";
            rsp["redirectUrl"] = "http://www.phiorg.ro/web_traffic_filtering/redirected_request";
            removeReq = true;
        }
    }

    return rsp.dump();
}

std::string Requests::Request::ScanEngineBeforeSendHeaders(bool& removeReq)
{
    removeReq = false;
    nlohmann::json rsp;
    rsp["status"] = "allow";

    if (m_url.has_value() &&
        *m_url == "http://www.phiorg.ro/web_traffic_filtering/modify_client_headers") {
        rsp["status"] = "block";
        rsp["headers"] = nlohmann::json::array();
        for (auto& h : m_requestHeaders) {
            nlohmann::json header = nlohmann::json::object();
            header["name"] = h.first;
            header["value"] = h.second;
            rsp["headers"].push_back(header);
        }

        // append a new header
        nlohmann::json header = nlohmann::json::object();
        header["name"] = "scan-engine";
        header["value"] = "demo";
        rsp["headers"].push_back(header);
    }

    return rsp.dump();
}

std::string Requests::Request::ScanEngineHeadersReceived(bool& removeReq)
{
    removeReq = false;
    nlohmann::json rsp;
    rsp["status"] = "allow";

    if (m_url.has_value() &&
        *m_url == "http://www.phiorg.ro/web_traffic_filtering/modify_server_headers") {
        rsp["status"] = "block";
        rsp["headers"] = nlohmann::json::array();
        for (auto& h : m_responseHeaders) {
            nlohmann::json header = nlohmann::json::object();
            header["name"] = h.first;
            header["value"] = h.second;
            rsp["headers"].push_back(header);
        }

        // append a new header
        nlohmann::json header = nlohmann::json::object();
        header["name"] = "Set-Cookie";
        header["value"] = "scan-engine=web-traffic-filtering-demo";
        rsp["headers"].push_back(header);
    }

    return rsp.dump();
}

static void replaceData(std::string& data, const std::string& oldSubstr, const std::string& newSubstr)
{
    size_t index = 0;
    while (true) {
        index = data.find(oldSubstr, index);
        if (index == std::string::npos)
            break;

        data.erase(index, oldSubstr.size());
        data.insert(index, newSubstr);

        index += newSubstr.size();
    }
}

std::string Requests::Request::ScanEngineResponseCompleted(bool& removeReq)
{
    removeReq = false;
    nlohmann::json rsp;
    rsp["status"] = "allowed";

    if (m_url.has_value()) {
        if (*m_url == "http://www.phiorg.ro/web_traffic_filtering/modify_server_response") {
            rsp["status"] = "blocked";
            std::string data = m_responseBody;
            replaceData(data, "test modify_server_response", "modified by web-traffic-filtering-demo");
            rsp["data"] = data;
        } else if (*m_url == "http://www.phiorg.ro/web_traffic_filtering/change_server_response") {
            rsp["status"] = "blocked";
            rsp["data"] = "blocked by web-traffic-filtering-demo";
        }
    }

    return rsp.dump();
}
