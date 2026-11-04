//
// Created by alberto on 12/8/26.
//

#pragma once

#include <stdexcept>
#include <source_location>
#include <string>

/**
 * @class EngineException
 * @brief Runtime error that records where it was thrown.
 *
 * The message is prefixed with `file:line:column [function]`, taken from
 * `std::source_location` at the construction site.
 */
class EngineException final : public std::runtime_error {
public:
    /**
     * @brief Builds the exception.
     * @param message  Human-readable description of the error.
     * @param location Throw site; defaults to the caller's location.
     */
    explicit EngineException(
        std::string message,
        std::source_location location =
            std::source_location::current()
    )
        : std::runtime_error(
            std::format(
                "{}:{}:{} [{}]: {}",
                location.file_name(),
                location.line(),
                location.column(),
                location.function_name(),
                message
            )
        ),
          file_(location.file_name()),
          line_(location.line()),
          function_(location.function_name()) {
    }

    /** @brief Source file where the exception was thrown. */
    [[nodiscard]]
    const char* file() const noexcept {
        return file_.c_str();
    }

    /** @brief Line where the exception was thrown. */
    [[nodiscard]]
    std::uint_least32_t line() const noexcept {
        return line_;
    }

    /** @brief Function where the exception was thrown. */
    [[nodiscard]]
    const char* function() const noexcept {
        return function_.c_str();
    }

private:
    std::string file_;            ///< Throw-site file name.
    std::uint_least32_t line_;    ///< Throw-site line.
    std::string function_;        ///< Throw-site function name.
};