//
// Created by alberto on 7/8/26.
//

#pragma once
#include <Engine/Defaults/DefaultConfig.h>
#include <Engine/Core/Logger/EngineException.h>

enum class LogLevel {
    Trace,
    Debug,
    Info,
    Warning,
    Error,
    Critical
};

class Logger {
public:
    static void Init();      // crear carpeta logs/ y abrir .log
    static void Shutdown();  // cerrar el archivo

    template<typename... Args>
    static void Console(LogLevel level, std::format_string<Args...> fmt, Args&&... args)
    {
        if (!Engine::Defaults::DEBUG_MODE && (level == LogLevel::Trace || level == LogLevel::Debug)) return;
        const auto line = formatLine(level, fmt, std::forward<Args>(args)...);
        std::lock_guard<std::mutex> lock(s_consoleMutex);
        std::cout << line << '\n';
    }

    template<typename... Args>
    static void File(LogLevel level, std::format_string<Args...> fmt, Args&&... args)
    {
        if (!Engine::Defaults::DEBUG_MODE && (level == LogLevel::Trace || level == LogLevel::Debug)) return;

        if (!s_logFile.is_open()) return;
        const auto line = formatLine(level, fmt, std::forward<Args>(args)...);
        std::lock_guard<std::mutex> lock(s_fileMutex);
        s_logFile << line << '\n';
    }

    template<typename... Args>
    static void Log(LogLevel level, std::format_string<Args...> fmt, Args&&... args)
    {
        if (!Engine::Defaults::DEBUG_MODE && (level == LogLevel::Trace || level == LogLevel::Debug)) return;
        const auto line = formatLine(level, fmt, std::forward<Args>(args)...);
        {
            std::lock_guard<std::mutex> lock(s_consoleMutex);
            std::cout << line << '\n';
        }
        {
            std::lock_guard<std::mutex> lock(s_fileMutex);
            if (s_logFile.is_open())
                s_logFile << line << '\n';
        }
    }

private:
    template<typename... Args>
    static std::string formatLine(LogLevel level, std::format_string<Args...> fmt, Args&&... args)
    {
        using namespace std::chrono;

        // Timestamp: [YYYY-MM-DD HH:MM:SS.ms]
        const auto now = system_clock::now();
        const auto t   = system_clock::to_time_t(now);
        const auto ms  = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

        std::tm tm{};
        localtime_r(&t, &tm);  // en Windows sería localtime_s

        char timeBuf[32];
        std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", &tm);

        // Mensaje formateado con std::format (C++20)
        const std::string msg = std::vformat(fmt.get(), std::make_format_args(args...));
        const auto levelStr = toString(level);

        return std::format("{}.{}\t[{}]\t {}",
                           timeBuf,
                           ms.count(),
                           levelStr,
                           msg);
    }

    static const char* toString(LogLevel level);

    // Estado global del logger
    static inline std::mutex     s_consoleMutex{};
    static inline std::mutex     s_fileMutex{};
    static inline std::ofstream  s_logFile{};
};
