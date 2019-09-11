#include "signatures.h"
#include "logger.h"

#include <fstream>

std::string to_string(const BeforeRequestStatus& status)
{
    std::string ret = "status = [" + status.status + "], ";
    ret += "redirectUrl = [" + status.redirectUrl + "]";
    return ret;
}

void Signatures::Init()
{
    std::ifstream file("../../service/signatures.json");
    if (file.is_open()) {
        try {
            nlohmann::json signatures = nlohmann::json::parse(file);
            LoadSignaturesBeforeRequest(signatures);
            LoadSignaturesBeforeSendHeaders(signatures);
            LoadSignaturesHeadersReceived(signatures);
            LoadSignaturesResponseCompleted(signatures);
        } catch (nlohmann::json::parse_error&) {
            LOG_ERROR("could not load signatures");
        }
        file.close();
    } else {
        LOG_ERROR("not found signatures");
    }
}

void Signatures::LoadSignaturesBeforeRequest(nlohmann::json& signatures)
{
    if (signatures.is_object() == false) {
        LOG_ERROR("signatures is not object");
        return;
    }

    auto beforeRequestSignatures = signatures.find("beforeRequest");
    if (beforeRequestSignatures == signatures.end()) {
        LOG_ERROR("could not find beforeRequest signatures");
        return;
    }

    if (beforeRequestSignatures->is_array() == false) {
        LOG_ERROR("beforeRequestSignatures is not array");
        return;
    }

    for (auto it = beforeRequestSignatures->begin(); it != beforeRequestSignatures->end(); ++it) {
        if (it->is_object() == false) {
            LOG_ERROR("beforeRequest signature is not object");
            continue;
        }

        if (it->find("url") == it->end()) {
            LOG_ERROR("beforeRequest signature has no url");
            continue;
        }

        if (it->find("status") == it->end()) {
            LOG_ERROR("beforeRequest signature has no status");
            continue;
        }

        BeforeRequestStatus status;
        std::string url = (*it)["url"];
        status.status = (*it)["status"];

        if (status.status == "redirect") {
            if (it->find("redirectUrl") == it->end()) {
                LOG_ERROR("beforeRequest signature has not redirectUrl");
                continue;
            }
            status.redirectUrl = (*it)["redirectUrl"];
        }

        LOG_INFO("found signature for url [%s] = [%s]",
            url.c_str(), to_string(status).c_str());
        m_beforeRequestSignatures[url] = status;
    }

    LOG_INFO("Loaded %s beforeRequest signatures",
        std::to_string(m_beforeRequestSignatures.size()).c_str());
}

void LoadHeadersField(nlohmann::json& headers, HeadersStatus& status)
{
    if (headers.is_array() == false) {
        LOG_ERROR("headers is not array");
        return;
    }

    for (auto it = headers.begin(); it != headers.end(); ++it) {
        if (it->is_object() == false) {
            LOG_ERROR("header is not object");
            continue;
        }
        HeaderStatus h;
        if (it->find("operation") == it->end()) {
            LOG_ERROR("header has no operation field");
            continue;
        }

        std::string op = (*it)["operation"];
        if (op == "add") {
            h.op = HeaderOperation::op_add;
        } else if (op == "remove") {
            h.op = HeaderOperation::op_remove;
        } else if (op == "modify") {
            h.op = HeaderOperation::op_modify;
        } else {
            LOG_ERROR("unknown operation [%s]", op.c_str());
            continue;
        }

        if (it->find("name") == it->end()) {
            LOG_ERROR("header has no name field");
            continue;
        }
        h.name = (*it)["name"];

        if (h.op != HeaderOperation::op_remove) {
            if (it->find("value") == it->end()) {
                LOG_ERROR("header has no value field");
                continue;
            }
            h.value = (*it)["value"];
        }

        status.headers.push_back(h);
    }
}

void Signatures::LoadSignaturesBeforeSendHeaders(nlohmann::json& signatures)
{
    if (signatures.is_object() == false) {
        LOG_ERROR("signatures is not object");
        return;
    }

    auto beforeSendHeaders = signatures.find("beforeSendHeaders");
    if (beforeSendHeaders == signatures.end()) {
        LOG_ERROR("could not find beforeSendHeaders signatures");
        return;
    }

    if (beforeSendHeaders->is_array() == false) {
        LOG_ERROR("beforeSendHeaders is not array");
        return;
    }

    for (auto it = beforeSendHeaders->begin(); it != beforeSendHeaders->end(); ++it) {
        if (it->is_object() == false) {
            LOG_ERROR("beforeSendHeaders signature is not object");
            continue;
        }

        if (it->find("url") == it->end()) {
            LOG_ERROR("beforeSendHeaders signature has no url");
            continue;
        }

        if (it->find("status") == it->end()) {
            LOG_ERROR("beforeSendHeaders signature has no status");
            continue;
        }

        HeadersStatus status;
        std::string url = (*it)["url"];
        status.status = (*it)["status"];

        if (it->find("originalHeaders") == it->end()) {
            status.originalHeaders = true;
        } else {
            status.originalHeaders = (*it)["originalHeaders"];
        }

        if (it->find("headers") != it->end()) {
            auto& headers = (*it)["headers"];
            LoadHeadersField(headers, status);
        }

        m_beforeSendHeaders[url] = status;
    }

    LOG_INFO("Loaded %s beforeSendHeaders signatures",
        std::to_string(m_beforeSendHeaders.size()).c_str());
}

void Signatures::LoadSignaturesHeadersReceived(nlohmann::json& signatures)
{
    if (signatures.is_object() == false) {
        LOG_ERROR("signatures is not object");
        return;
    }

    auto headersReceived = signatures.find("headersReceived");
    if (headersReceived == signatures.end()) {
        LOG_ERROR("could not find headersReceived signatures");
        return;
    }

    if (headersReceived->is_array() == false) {
        LOG_ERROR("headersReceived is not array");
        return;
    }

    for (auto it = headersReceived->begin(); it != headersReceived->end(); ++it) {
        if (it->is_object() == false) {
            LOG_ERROR("headersReceived signature is not object");
            continue;
        }

        if (it->find("url") == it->end()) {
            LOG_ERROR("headersReceived signature has no url");
            continue;
        }

        if (it->find("status") == it->end()) {
            LOG_ERROR("headersReceived signature has no status");
            continue;
        }

        HeadersStatus status;
        std::string url = (*it)["url"];
        status.status = (*it)["status"];

        if (it->find("originalHeaders") == it->end()) {
            status.originalHeaders = true;
        } else {
            status.originalHeaders = (*it)["originalHeaders"];
        }

        if (it->find("headers") != it->end()) {
            auto& headers = (*it)["headers"];
            LoadHeadersField(headers, status);
        }

        m_headersReceived[url] = status;
    }

    LOG_INFO("Loaded %s headersReceived signatures",
        std::to_string(m_headersReceived.size()).c_str());
}

void Signatures::LoadSignaturesResponseCompleted(nlohmann::json& signatures)
{
    if (signatures.is_object() == false) {
        LOG_ERROR("signatures is not object");
        return;
    }

    auto responseCompleted = signatures.find("responseCompleted");
    if (responseCompleted == signatures.end()) {
        LOG_ERROR("could not find responseCompleted signatures");
        return;
    }

    if (responseCompleted->is_array() == false) {
        LOG_ERROR("responseCompleted is not array");
        return;
    }

    for (auto it = responseCompleted->begin(); it != responseCompleted->end(); ++it) {
        if (it->is_object() == false) {
            LOG_ERROR("responseCompleted signature is not object");
            continue;
        }

        if (it->find("url") == it->end()) {
            LOG_ERROR("responseCompleted signature has no url");
            continue;
        }

        if (it->find("status") == it->end()) {
            LOG_ERROR("responseCompleted signature has no status");
            continue;
        }

        ResponseCompletedStatus status;
        std::string url = (*it)["url"];
        status.status = (*it)["status"];

        if (status.status != "allow") {
            if (it->find("new_data") == it->end()) {
                LOG_ERROR("responseCompleted signature has no new_data field");
                continue;
            }
            status.newData = (*it)["new_data"];

            if (status.status == "modify") {
                if (it->find("old_data") == it->end()) {
                    LOG_ERROR("responseCompleted signature has no old_data field");
                    continue;
                }
                status.oldData = (*it)["old_data"];
            }
        }

        m_responseCompleted[url] = status;
    }

    LOG_INFO("Loaded %s responseCompleted signatures",
        std::to_string(m_responseCompleted.size()).c_str());
}

bool Signatures::MatchSignatureBeforeRequest(const std::string& url,
    BeforeRequestStatus& status) const
{
    auto found = m_beforeRequestSignatures.find(url);
    if (found == m_beforeRequestSignatures.end()) {
        return false;
    }

    status.status = found->second.status;
    status.redirectUrl = found->second.redirectUrl;
    return true;
}

bool Signatures::MatchSignatureBeforeSendHeaders(const std::string& url,
    HeadersStatus& status) const
{
    auto found = m_beforeSendHeaders.find(url);
    if (found == m_beforeSendHeaders.end()) {
        return false;
    }

    status.status = found->second.status;
    status.originalHeaders = found->second.originalHeaders;
    status.headers = found->second.headers;
    return true;
}

bool Signatures::MatchSignatureHeadersReceived(const std::string& url,
    HeadersStatus& status) const
{
    auto found = m_headersReceived.find(url);
    if (found == m_headersReceived.end()) {
        return false;
    }

    status.status = found->second.status;
    status.originalHeaders = found->second.originalHeaders;
    status.headers = found->second.headers;
    return true;
}

bool Signatures::MatchSignatureResponseCompleted(const std::string& url,
    ResponseCompletedStatus& status) const
{
    auto found = m_responseCompleted.find(url);
    if (found == m_responseCompleted.end()) {
        return false;
    }

    status.status = found->second.status;
    status.oldData = found->second.oldData;
    status.newData = found->second.newData;
    return true;
}