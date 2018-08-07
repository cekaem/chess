/* Component tests for class UCIHandler */

#include <chrono>
#include <condition_variable>
#include <exception>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <sstream>
#include <thread>
#include <unistd.h>

#include "utils/Test.h"
#include "Board.h"
#include "Logger.h"
#include "Engine.h"
#include "Field.h"
#include "Figure.h"
#include "UCIHandler.h"


namespace {

class UCIHandlerWrapper {
 public:
  bool sendCommandAndWaitForResponse(const std::string& command,
                                     const std::string& response,
                                     unsigned timeout) {
    std::stringstream ss;
    UCIHandler handler(ss, ss);
    std::cout << "MAIN: starting thread." << std::endl;
    std::thread t(&UCIHandlerWrapper::uciThread, this, std::ref(handler));
    t.detach();
    std::unique_lock ul(uci_handler_started_mutex_);
    uci_handler_started_cv_.wait(ul, [this] { return uci_handler_started_ == true; });
    std::cout << "MAIN: thread started." << std::endl;
    auto start_time = std::chrono::steady_clock::now();
    unsigned time_elapsed = 0;
    ss << command << std::endl;
    std::cout << "MAIN: command sent" << std::endl;
    bool response_found = false;
    do {
      std::string s;
      while (ss >> s) {
        if (s.find(response) != std::string::npos) {
          std::cout << "MAIN: found response" << std::endl;
          response_found = true;
          break;
        }
      }
      if (response_found == true) {
        break;
      }
      usleep(5000);
      auto end_time = std::chrono::steady_clock::now();
      time_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    } while (time_elapsed < timeout);

    std::cout << "MAIN: sending quit" << std::endl;
    
    ss << "quit" << std::endl;
    ul.lock();
    std::cout << "MAIN: waiting for thread to finish" << std::endl;
    uci_handler_started_cv_.wait(ul, [this] { return uci_handler_started_ == false; });

    std::cout << "MAIN: end" << std::endl;
    
    return response_found;
  }
  
 private:
  void uciThread(UCIHandler& handler) {
    {
      std::unique_lock ul(uci_handler_started_mutex_);
      uci_handler_started_ = true;
    }
    std::cout << "THREAD: started" << std::endl;
    uci_handler_started_cv_.notify_one();
    handler.start();
    std::cout << "THREAD: uci handler ended" << std::endl;
    std::unique_lock ul(uci_handler_started_mutex_);
    uci_handler_started_ = false;
    uci_handler_started_cv_.notify_one();
    std::cout << "THREAD: end" << std::endl;
  }

  bool uci_handler_started_{false};
  std::mutex uci_handler_started_mutex_;
  std::condition_variable uci_handler_started_cv_;
};
    

// Checks if UCIHandler properly handles unrecognized commands
TEST_PROCEDURE(test1) {
  TEST_START
  std::stringstream ss;
  UCIHandler handler(ss, ss);
  try {
    handler.handleCommand("invalid_command");
  } catch (const UCIHandler::UnknownCommandException& e) {
    VERIFY_STRINGS_EQUAL("invalid_command", e.command.c_str());
    RETURN
  }
  NOT_REACHED
  TEST_END
}

// Checks if UCIHandler properly handles command uci
TEST_PROCEDURE(test2) {
  TEST_START
  UCIHandlerWrapper wrapper;
  Logger::getLogger().start(9090, Logger::LogSection::UCI_HANDLER);
  wrapper.sendCommandAndWaitForResponse("uci", "uciok", 100);
  TEST_END
}

} // unnamed namespace


int main() {
  try {
    TEST("UCIHandler properly handles unrecognized commands", test1);
    TEST("UCIHandler properly handles command uci", test2);
  } catch (std::exception& except) {
    std::cerr << "Unexpected exception: " << except.what() << std::endl;
     return -1;
  }
  int failed_tests = Test::get_number_of_failed_tests();
  if (failed_tests > 0) {
    std::cout << failed_tests << " test(s) failed." << std::endl;
    return -2;
  }
  std::cout << "All tests passed." << std::endl;
  return 0;
}
