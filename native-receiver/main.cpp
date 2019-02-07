#include "logger.h"
#include "extension_comm.h"
#include "service_comm.h"

#include <nlohmann/json.hpp>

int main(int argc, const char* argv[])
{
    int ret = 1;
    ExtensionComm extensionComm;
    ServiceComm serviceComm;
    std::string data, serviceRsp;
    nlohmann::json rsp;
    Logger::LoggerConfig config = { "native-receiver", false, false, true, true, true, true };

    logger.Init(config);

    LOG_INFO("native-receiver started");

    do {
        rsp["error"] = "could not communicate with the service";

        if (extensionComm.RecvFromExtension(data) == false) {
            LOG_ERROR("could not get the message");
            rsp["error"] = "could not get the message from extension";
            break;
        }

        if (serviceComm.Connect() == false) {
            LOG_ERROR("could not connect to service");
            rsp["error"] = "could not connect to service";
            break;
        }

        bool sendRet = serviceComm.Send(data, serviceRsp);
        serviceComm.Disconnect();

        if (sendRet == false) {
            LOG_ERROR("could not send data to service");
            rsp["error"] = "could not send data to service";
            break;
        }

        LOG_INFO("received response from service [%s]", serviceRsp.c_str());
        try {
            rsp = nlohmann::json::parse(serviceRsp);
        } catch (nlohmann::json::parse_error&) {
            LOG_ERROR("could not parse response from service");
            rsp = nlohmann::json();
            rsp["error"] = "could not parse response from service";
            break;
        }

        ret = 0;
    } while (false);

    extensionComm.SendToExtension(rsp.dump());

    LOG_INFO("native-receiver exited with [%d]", ret);
    return ret;
}