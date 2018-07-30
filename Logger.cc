#include "Logger.h"

#include <condition_variable>
#include <exception>
#include <mutex>
#include <thread>
#include <unistd.h>

#include "utils/Utils.h"


Logger& Logger::getLogger() {
  static Logger logger;
  return logger;
}

Logger::~Logger() {
  do_memory_consumption_measures_ = false;
  std::unique_lock<std::mutex> ul(memory_consumption_measures_ended_mutex_);
  memory_consumption_measures_ended_cv_.wait(
    ul, [this] { return memory_consumption_measures_ended_ == true; });
}

void Logger::start(int port, Logger::LogSection log_sections_mask) {
  if (is_started_ == true) {
    throw std::runtime_error("Logger already started");
  }
  is_started_ = true;
  log_sections_mask_ = log_sections_mask;
  log_.waitForClient(port);
  if ((log_sections_mask & LogSection::MEMORY_CONSUMPTION) != LogSection::NONE) {
    memory_consumption_measures_ended_ = false;
    std::thread memory_consumption_logger(&Logger::logMemoryConsumption,
                                          this,
                                          SecondsBetweenMemoryConsumptionMeasures);
    memory_consumption_logger.detach();
  }
}

bool Logger::shouldLog(Logger::LogSection section) const {
  return (section & log_sections_mask_) != LogSection::NONE;
}

void Logger::logMemoryConsumption(int sec) {
  unsigned max_consumption = 0u;
  unsigned long long total_consumption = 0u;
  unsigned number_of_measures = 0u;
  while (do_memory_consumption_measures_ == true) {
    unsigned memory_consumption = 0;
    try {
      memory_consumption = utils::getMemoryUsageOfCurrentProcess();
    } catch (const std::exception& e) {
      LogWithEndLine(Logger::LogSection::MEMORY_CONSUMPTION, e.what());
      do_memory_consumption_measures_ = false;
      break;
    }
    LogWithEndLine(LogSection::MEMORY_CONSUMPTION, "Memory consumption: ", memory_consumption);
    total_consumption += memory_consumption;
    ++number_of_measures;
    if (memory_consumption > max_consumption) {
      max_consumption = memory_consumption;
    }
    ::sleep(sec);
  }
  LogWithEndLine(LogSection::MEMORY_CONSUMPTION, "Maximum consumption: ", max_consumption);
  LogWithEndLine(LogSection::MEMORY_CONSUMPTION, "Average consumption: ", total_consumption / number_of_measures);

  memory_consumption_measures_ended_ = true;
  memory_consumption_measures_ended_cv_.notify_one();
}
