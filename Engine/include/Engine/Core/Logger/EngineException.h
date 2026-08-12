//
// Created by alberto on 12/8/26.
//

#pragma once

#include <stdexcept>
#include <source_location>
#include <string>

class EngineException final : public std::runtime_error {
public:
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

    [[nodiscard]]
    const char* file() const noexcept {
        return file_.c_str();
    }

    [[nodiscard]]
    std::uint_least32_t line() const noexcept {
        return line_;
    }

    [[nodiscard]]
    const char* function() const noexcept {
        return function_.c_str();
    }

private:
    std::string file_;
    std::uint_least32_t line_;
    std::string function_;
};