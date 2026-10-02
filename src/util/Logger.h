#pragma once

#include <atomic>
#include <iostream>
#include <mutex>
#include <string>
#include <utility>

class Logger {
public:
  enum class Level { DEBUG, INFO, WARN, ERROR };

  static void setLevel(Level lv) { level_.store(lv); }

  template <typename... Args> static void debug(Args &&...args) {
    log(Level::DEBUG, std::forward<Args>(args)...);
  }
  template <typename... Args> static void info(Args &&...args) {
    log(Level::INFO, std::forward<Args>(args)...);
  }
  template <typename... Args> static void warn(Args &&...args) {
    log(Level::WARN, std::forward<Args>(args)...);
  }
  template <typename... Args> static void error(Args &&...args) {
    log(Level::ERROR, std::forward<Args>(args)...);
  }

private:
  template <typename... Args> static void log(Level lv, Args &&...args) {
    if (lv < level_.load())
      return;

    std::lock_guard<std::mutex> lock(mtx_); // 共享锁
    std::cout << "[" << levelTag(lv) << "]";
    (std::cout << ... << args); // C++17 折叠表达式
    std::cout << "\n";
  }

  static const char *levelTag(Level lv) {
    switch (lv) {
    case Level::DEBUG:
      return "DEBUG";
    case Level::INFO:
      return "INFO";
    case Level::WARN:
      return "WARN";
    case Level::ERROR:
      return "ERROR";
    }
    return "????";
  }

  inline static std::atomic<Level> level_{Level::INFO};
  inline static std::mutex mtx_;
};