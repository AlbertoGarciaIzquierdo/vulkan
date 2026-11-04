//
// Created by alberto on 7/8/26.
//

#pragma once
#include <Engine/Defaults/DefaultConfig.h>
#include <Engine/Core/Logger/EngineException.h>

/** @brief Severity of a log message, from least to most severe. */
enum class LogLevel {
    Trace,
    Debug,
    Info,
    Warning,
    Error,
    Critical
};

/** @brief ANSI escape sequences used to color console output. */
namespace ConsoleColor
{
    inline constexpr std::string_view Reset =
        "\x1B[0m";

    inline constexpr std::string_view White =
        "\x1B[0m";

    inline constexpr std::string_view Grey =
        "\x1B[37m";

    inline constexpr std::string_view Yellow =
        "\x1B[33m";

    inline constexpr std::string_view Red =
        "\x1B[31m";

    inline constexpr std::string_view Green =
        "\x1B[32m";

    inline constexpr std::string_view BrightRed =
        "\x1B[1;31m";
}

/**
 * @class Logger
 * @brief Thread-safe static logger that writes to the console and to a daily log file.
 *
 * Messages use `std::format` syntax. Trace and Debug messages are discarded
 * when `Engine::Defaults::DEBUG_MODE` is false.
 */
class Logger {
public:
    /** @brief Opens `logs/Engine_YYYYMMDD.log` in append mode and logs a start marker. */
    static void Init();
    /** @brief Logs an end marker and closes the log file. */
    static void Shutdown();

    /**
     * @brief Maps a log level to its console color.
     * @param level Severity of the message.
     * @return ANSI escape sequence for that level.
     */
    static std::string_view colorForLevel(LogLevel level)
    {
        switch (level)
        {
        case LogLevel::Trace:
        case LogLevel::Debug:
            return ConsoleColor::Grey;

        case LogLevel::Info:
            return ConsoleColor::White;

        case LogLevel::Warning:
            return ConsoleColor::Yellow;

        case LogLevel::Error:
            return ConsoleColor::Red;

        case LogLevel::Critical:
            return "\033[1;31m"; // Bright red
        }

        return ConsoleColor::Reset;
    }

    /**
     * @brief Writes a formatted message to the console only.
     * @param level Severity of the message.
     * @param fmt   `std::format` string.
     * @param args  Values to format.
     */
    template<typename... Args>
    static void Console(LogLevel level, std::format_string<Args...> fmt, Args&&... args)
    {
        if (!Engine::Defaults::DEBUG_MODE && (level == LogLevel::Trace || level == LogLevel::Debug)) return;

        const auto line = formatLine(level, fmt, std::forward<Args>(args)...);

        const auto color = colorForLevel(level);

        std::lock_guard<std::mutex> lock(s_consoleMutex);

        std::print("{}{}{}\n", color, line, ConsoleColor::Reset);
    }

    /**
     * @brief Writes a formatted message to the log file only (ignored if the file is not open).
     * @param level Severity of the message.
     * @param fmt   `std::format` string.
     * @param args  Values to format.
     */
    template<typename... Args>
    static void File(LogLevel level, std::format_string<Args...> fmt, Args&&... args)
    {
        if (!Engine::Defaults::DEBUG_MODE && (level == LogLevel::Trace || level == LogLevel::Debug)) return;

        if (!s_logFile.is_open()) return;
        const auto line = formatLine(level, fmt, std::forward<Args>(args)...);
        std::lock_guard<std::mutex> lock(s_fileMutex);
        s_logFile << line << '\n';
    }

    /**
     * @brief Writes a formatted message to both the console and the log file.
     * @param level Severity of the message.
     * @param fmt   `std::format` string.
     * @param args  Values to format.
     */
    template<typename... Args>
    static void Log(LogLevel level, std::format_string<Args...> fmt, Args&&... args)
    {
        if (!Engine::Defaults::DEBUG_MODE && (level == LogLevel::Trace || level == LogLevel::Debug)) return;
        const auto line = formatLine(level, fmt, std::forward<Args>(args)...);
        {
            std::lock_guard<std::mutex> lock(s_consoleMutex);
            const auto color = colorForLevel(level);
            std::print("{}{}{}\n", color, line, ConsoleColor::Reset);
        }
        {
            std::lock_guard<std::mutex> lock(s_fileMutex);
            if (s_logFile.is_open())
                s_logFile << line << '\n';
        }
    }

private:
    /** @brief Builds `timestamp<TAB>[LEVEL]<TAB> message` from a format string. */
    template<typename... Args>
    static std::string formatLine(LogLevel level, std::format_string<Args...> fmt, Args&&... args)
    {
        using namespace std::chrono;

        // Timestamp: [YYYY-MM-DD HH:MM:SS.ms]
        const auto now = system_clock::now();
        const auto t   = system_clock::to_time_t(now);
        const auto ms  = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

        std::tm tm{};
        localtime_r(&t, &tm);  // On Windows this would be localtime_s

        char timeBuf[32];
        std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", &tm);

        const std::string msg = std::vformat(fmt.get(), std::make_format_args(args...));
        const auto levelStr = toString(level);

        return std::format("{}.{}\t[{}]\t {}",
                           timeBuf,
                           ms.count(),
                           levelStr,
                           msg);
    }

    /** @brief Returns the upper-case name of a log level. */
    static const char* toString(LogLevel level);

    // Global logger state
    static inline std::mutex     s_consoleMutex{};  ///< Serializes console output.
    static inline std::mutex     s_fileMutex{};     ///< Serializes file output.
    static inline std::ofstream  s_logFile{};       ///< Daily log file.
};
