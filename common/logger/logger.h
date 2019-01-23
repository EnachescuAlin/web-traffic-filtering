#include <cstdio>
#include <string>

#include <sys/types.h>

class Logger {
public:
    Logger();
    ~Logger();

    struct LoggerConfig {
        const char *filename = nullptr;
        bool appendDateAndTimeToFilename = false;;
        bool appendPidToFilename = false;
        bool appendOpenMode = false;
        bool logDateAndTime = false;
        bool logPid = false;
        bool logTid = false;
    };

    void Init(const LoggerConfig& config);
    void Log(const char *functionName, const char *logLevel, const char *format, ...);

private:
    std::string _GetLogFilename();

private:
    FILE *m_file = nullptr;
    LoggerConfig m_config;
};

#ifndef LOGGER_CPP_FILE
extern Logger logger;
#endif

#define LOG_ERROR(fmt, ...) logger.Log(__FUNCTION__, "ERROR", fmt, ##__VA_ARGS__)
#define LOG_WARNING(fmt, ...) logger.Log(__FUNCTION__, "WARNING", fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) logger.Log(__FUNCTION__, "INFO", fmt, ##__VA_ARGS__)
