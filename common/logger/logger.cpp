#define LOGGER_CPP_FILE
#include "logger.h"

#include <ctime>
#include <cstdarg>

#include <unistd.h>
#include <sys/syscall.h>

Logger logger;

Logger::Logger()
{
}

Logger::~Logger()
{
    if (m_file != nullptr) {
        fclose(m_file);
        m_file = nullptr;
    }
}

void Logger::Init(const LoggerConfig& config)
{
    m_config = config;

    std::string filename = _GetLogFilename();
    if (m_config.appendOpenMode) {
        m_file = fopen(filename.c_str(), "a");
    } else {
        m_file = fopen(filename.c_str(), "w");
    }
}

void Logger::Log(const char *functionName, const char *logLevel, const char *format, ...)
{
    std::string output;

    if (m_config.logDateAndTime) {
        char dateAndTimeBuffer[1024];
        time_t currentTime = time(0);
        strftime(dateAndTimeBuffer, 1024, "[%d-%m-%Y %H:%M:%S] ", localtime(&currentTime));
        output.append(dateAndTimeBuffer);
    }

    if (m_config.logPid) {
        output.append("[pid_" + std::to_string(getpid()) + "] ");
    }

    if (m_config.logTid) {
        output.append("[tid_" + std::to_string(syscall(SYS_gettid)) + "] ");
    }

    if (functionName) {
        output += "[";
        output += functionName;
        output += "] ";
    }

    if (logLevel) {
        output += "[";
        output += logLevel;
        output += "] ";
    }

    {
        char formatBuffer[4096];
        va_list args;

        va_start(args, format);
        vsnprintf(formatBuffer, 4096, format, args);
        va_end(args);

        output.append(formatBuffer);
    }

    output.append("\n");
    fputs(output.c_str(), m_file);
    fflush(m_file);
}

std::string Logger::_GetLogFilename()
{
    std::string filename = m_config.filename;

    if (m_config.appendDateAndTimeToFilename) {
        char dateAndTimeBuffer[1024];
        time_t currentTime = time(0);
        strftime(dateAndTimeBuffer, 1024, "__%d-%m-%Y__%H-%M-%S", localtime(&currentTime));
        filename.append(dateAndTimeBuffer);
    }

    if (m_config.appendPidToFilename) {
        filename.append("__");
        filename.append(std::to_string(getpid()));
    }

    filename.append(".log");

    return filename;
}
