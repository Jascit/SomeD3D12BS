/* SPDX-License-Identifier: Apache-2.0 */

/**
 * @file    log.cpp
 * @brief
 *
 * @date    02/09/2026
 *
 * @copyright Copyright (c) 2026 The Project Contributors
 */
module;
#include <chrono>
#include <format>
#include <iostream>
#include <filesystem>

module core.log;
using namespace core::diagnostics;

namespace {
auto get_level_string(Level level) noexcept -> const char * {
  switch (level) {
    case Level::Error:
      return "ERROR";
    case Level::Warning:
      return "WARNING";
    case Level::Info:
      return "INFO";
    case Level::Debug:
      return "DEBUG";
    case Level::Fatal:
      return "FATAL";
    case Level::Trace:
      return "TRACE";
    default:
      return "DEBUG";
  }
}
} // namespace

void ConsoleSink::write(const Event &event) {
  const std::string message =
      std::format("[{:%Y-%m-%d %H:%M:%S}] [{}] [{}:{}] {}\n", event.timestamp, get_level_string(event.level),
                  event.location.file_name(), event.location.line(), event.message);
  std::cout << message;
}

FileSink::FileSink(std::filesystem::path path)
    : path_(std::move(path)) {
  file_.open(path_, std::ios::out | std::ios::trunc);
  if (!file_.is_open()) {
    std::cerr << "Failed to open file: " << path_ << '\n';
  }
}

void FileSink::write(const Event &event) {
  if (!file_.is_open()) {
    return;
  }
  const std::string message =
      std::format("[{:%Y-%m-%d %H:%M:%S}] [{}] [{}:{}] {}\n", event.timestamp, get_level_string(event.level),
                  event.location.file_name(), event.location.line(), event.message);
  file_ << message;
  if (event.level >= Level::Error) {
    file_.flush();
  }
}

void Logger::write(const Event &event) {
  if (sinks_.empty()) {
    std::cerr << "[{}] No sinks registered\n";
    return;
  }
  for (auto &sink : sinks_) {
    sink->write(event);
  }
}

void Logger::register_sink(std::unique_ptr<ISink> sink) { sinks_.push_back(std::move(sink)); }

auto Logger::instance() noexcept -> Logger & {
  static Logger logger;
  return logger;
}
