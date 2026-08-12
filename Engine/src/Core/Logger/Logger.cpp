//
// Created by alberto on 7/8/26.
//

#include <Engine/Core/Logger/Logger.h>

#include "Engine/Defaults/DefaultConfig.h"

void Logger::Init()
{
    namespace fs = std::filesystem;

    const fs::path logDir{"logs"};
    fs::create_directories(logDir);

    const auto now = std::chrono::system_clock::now();
    const auto t   = std::chrono::system_clock::to_time_t(now);

    std::tm tm{};
    localtime_r(&t, &tm);
    char dateBuf[16];
    std::strftime(dateBuf, sizeof(dateBuf), "%Y%m%d", &tm);

    fs::path logPath = logDir / std::format("Engine_{}.log", dateBuf);

    s_logFile.open(logPath, std::ios::app);

    Log(LogLevel::Info, "-------------------------------- Process started ---------------------------------------");
}

void Logger::Shutdown()
{
    Log(LogLevel::Info, "-------------------------------- Process finished ---------------------------------------");
    if (s_logFile.is_open())
    {
        s_logFile.close();
    }
}

const char* Logger::toString(LogLevel level)
{
    switch (level) {
    case LogLevel::Trace:    return "TRACE";
    case LogLevel::Debug:    return "DEBUG";
    case LogLevel::Info:     return "INFO";
    case LogLevel::Warning:  return "WARNING";
    case LogLevel::Error:    return "ERROR";
    case LogLevel::Critical: return "CRITICAL";
    default:                 return "UNKNOWN";
    }
}