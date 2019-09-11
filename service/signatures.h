#ifndef SIGNATURES_H
#define SIGNATURES_H

#include <string>
#include <map>
#include <nlohmann/json.hpp>

enum HeaderOperation {
    op_add,
    op_remove,
    op_modify
};

struct HeaderStatus {
    std::string name;
    std::string value;
    HeaderOperation op;
};

struct BeforeRequestStatus {
    std::string status;
    std::string redirectUrl;
};

struct HeadersStatus {
    std::string status;
    bool originalHeaders;
    std::vector<HeaderStatus> headers;
};

struct ResponseCompletedStatus {
    std::string status;
    std::string oldData;
    std::string newData;
};

class Signatures {
public:
    Signatures() = default;
    ~Signatures() = default;

    void Init();

    bool MatchSignatureBeforeRequest(const std::string&, BeforeRequestStatus&) const;
    bool MatchSignatureBeforeSendHeaders(const std::string&, HeadersStatus&) const;
    bool MatchSignatureHeadersReceived(const std::string&, HeadersStatus&) const;
    bool MatchSignatureResponseCompleted(const std::string&, ResponseCompletedStatus&) const;

private:
    void LoadSignaturesBeforeRequest(nlohmann::json& signatures);
    void LoadSignaturesBeforeSendHeaders(nlohmann::json& signatures);
    void LoadSignaturesHeadersReceived(nlohmann::json& signatures);
    void LoadSignaturesResponseCompleted(nlohmann::json& signatures);

private:
    std::map<std::string, BeforeRequestStatus> m_beforeRequestSignatures;
    std::map<std::string, HeadersStatus> m_beforeSendHeaders;
    std::map<std::string, HeadersStatus> m_headersReceived;
    std::map<std::string, ResponseCompletedStatus> m_responseCompleted;
};

#endif
