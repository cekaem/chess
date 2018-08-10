#ifndef LOGGER_H
#define LOGGER_H

#include <condition_variable>
#include <functional>
#include <mutex>

#include "utils/SocketLog.h"

class Logger {
 public:
  static constexpr int SecondsBetweenMemoryConsumptionMeasures = 1;

  enum class LogSection {
    NONE = 0x0,
    ENGINE_MOVE_SEARCHES = 0x01,
    ENGINE_MATES = 0x02,
    ENGINE_THREADS = 0x04,
    ENGINE_TIMER = 0x08,
    MEMORY_CONSUMPTION = 0x10,
    UCI_HANDLER = 0x20,
    ALL = ENGINE_MOVE_SEARCHES |
          ENGINE_MATES |
          ENGINE_THREADS |
          ENGINE_TIMER |
          MEMORY_CONSUMPTION |
          UCI_HANDLER
  };

  ~Logger();
  static Logger& getLogger();
  void start(int port, LogSection log_sections_mask);
  static bool shouldLog(LogSection section);
  bool alertOnMemoryConsumption(unsigned threshold, std::function<void(int)> callback);
  utils::SocketLog& getStream() { return log_; }

 private:
  Logger() = default;
  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;

  void startLoggingMemoryConsumption();
  void logMemoryConsumption();

  utils::SocketLog log_;
  bool is_started_{false};
  static LogSection log_sections_mask_;

  bool do_memory_consumption_measures_{true};
  bool memory_consumption_measures_ended_{true};
  bool is_logging_memory_consumption_{false};
  unsigned memory_consumption_threshold_{0u};
  std::function<void(int)> memory_consumption_callback_;
  std::mutex memory_consumption_measures_ended_mutex_;
  std::mutex do_memory_consumption_measures_mutex_;
  std::condition_variable memory_consumption_measures_ended_cv_;
  std::condition_variable do_memory_consumption_measures_cv_;
};

inline Logger::LogSection operator&(
    Logger::LogSection l,
    Logger::LogSection r) {
  return static_cast<Logger::LogSection>(
      static_cast<unsigned>(l) & static_cast<unsigned>(r));
}

inline Logger::LogSection operator|(
    Logger::LogSection l,
    Logger::LogSection r) {
  return static_cast<Logger::LogSection>(
      static_cast<unsigned>(l) | static_cast<unsigned>(r));
}

template<typename T1>
void LogWithEndLine(Logger::LogSection section, const T1& t1) {
  if (Logger::getLogger().shouldLog(section)) {
    Logger::getLogger().getStream() << utils::SocketLog::lock << t1 << utils::SocketLog::endl;
  }
}

template<typename T1, typename T2>
void LogWithEndLine(Logger::LogSection section,
                          const T1& t1,
                          const T2& t2) {
  if (Logger::getLogger().shouldLog(section)) {
    Logger::getLogger().getStream() << utils::SocketLog::lock << t1 << t2 << utils::SocketLog::endl;
  }
}

template<typename T1, typename T2, typename T3>
void LogWithEndLine(Logger::LogSection section,
                          const T1& t1,
                          const T2& t2,
                          const T3& t3) {
  if (Logger::getLogger().shouldLog(section)) {
    Logger::getLogger().getStream() << utils::SocketLog::lock << t1 << t2 << t3 << utils::SocketLog::endl;
  }
}

template<typename T1>
void Log(Logger::LogSection section, const T1& t1) {
  if (Logger::getLogger().shouldLog(section)) {
    Logger::getLogger().getStream() << t1;
  }
}

template<typename T1, typename T2>
void Log(Logger::LogSection section,
         const T1& t1,
         const T2& t2) {
  if (Logger::getLogger().shouldLog(section)) {
    Logger::getLogger().getStream() << t1 << t2;
  }
}

template<typename T1, typename T2, typename T3>
void Log(Logger::LogSection section,
         const T1& t1,
         const T2& t2,
         const T3& t3) {
  if (Logger::getLogger().shouldLog(section)) {
    Logger::getLogger().getStream() << t1 << t2 << t3;
  }
}

#endif  // LOGGER_H
