/* SPDX-License-Identifier: Apache-2.0 */
/**
 * @file    log.cppm
 * @brief   Core diagnostics and logging system
 * @date    02/09/2026
 * @copyright Copyright (c) 2026 The Project Contributors
 */
module;
#include <filesystem>
#include <fstream>
#include <source_location>
#include <vector>

export module core.log;

export namespace core::diagnostics
{

enum class Level
{
    Trace,
    Debug,
    Info,
    Warning,
    Error,
    Fatal
};

struct Event
{
    Level level;
    std::string message;
    std::chrono::system_clock::time_point timestamp;
    std::source_location location;
};

class ISink
{
  public:
    virtual ~ISink() = default;
    virtual void write(const Event &event) = 0;
};

class ConsoleSink : public ISink
{
  public:
    ~ConsoleSink() override = default;
    void write(const Event &event) override;
};

class FileSink : public ISink
{
  public:
    explicit FileSink(std::filesystem::path path);
    ~FileSink() override = default;
    void write(const Event &event) override;

  private:
    std::ofstream file_;
    std::filesystem::path path_;
};

class Logger
{
  public:
    static auto instance() noexcept -> Logger &;
    Logger(const Logger &) = delete;
    Logger(Logger &&) = delete;
    auto operator=(const Logger &) -> Logger & = delete;
    auto operator=(Logger &&) -> Logger & = delete;

    ~Logger() = default;

    void write(const Event &event);

    void register_sink(std::unique_ptr<ISink> sink);

  private:
    Logger() = default;
    std::vector<std::unique_ptr<ISink>> sinks_;
};

namespace detail
{
void log_impl(Level level, std::string message, std::source_location loc)
{
    Event event{
        .level = level, .message = std::move(message), .timestamp = std::chrono::system_clock::now(), .location = loc};
    Logger::instance().write(event);
}

template <typename... Args> struct LogMessage
{
    std::format_string<Args...> fmt;
    std::source_location loc;

    template <typename String>
    consteval LogMessage(const String &s, std::source_location l = std::source_location::current()) : fmt{s}, loc{l}
    {
    }
};
#ifdef DEBUG
template <typename... Args> void log_debug(std::type_identity_t<LogMessage<Args...>> log_massage, Args &&...args)
{
    detail::log_impl(Level::Debug, std::format(log_massage.fmt, std::forward<Args>(args)...), log_massage.loc);
}
#endif
} // namespace detail

template <typename... Args> void log_info(std::type_identity_t<detail::LogMessage<Args...>> log_massage, Args &&...args)
{
    detail::log_impl(Level::Info, std::format(log_massage.fmt, std::forward<Args>(args)...), log_massage.loc);
}

template <typename... Args>
void log_warning(std::type_identity_t<detail::LogMessage<Args...>> log_massage, Args &&...args)
{
    detail::log_impl(Level::Warning, std::format(log_massage.fmt, std::forward<Args>(args)...), log_massage.loc);
}

template <typename... Args>
void log_error(std::type_identity_t<detail::LogMessage<Args...>> log_massage, Args &&...args)
{
    detail::log_impl(Level::Error, std::format(log_massage.fmt, std::forward<Args>(args)...), log_massage.loc);
}

} // namespace core::diagnostics
