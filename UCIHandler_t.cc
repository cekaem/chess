/* Component tests for class UCIHandler */

#include <cassert>
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

class LineStreamBuf : public std::streambuf {
 public:
  bool isLineReadyForRead() const { return line_completed_; }

 private:
  int overflow(int c) override {
    std::unique_lock<std::mutex> ul(line_completed_mutex_);
    line_completed_cv_.wait(ul, [this] { return line_completed_ == false; });
    buff_.push_back(c);
    if (c == '\n') {
      line_completed_ = true;
      line_completed_cv_.notify_one();
    }
    return c;
  }

  int underflow() override {
    std::unique_lock<std::mutex> ul(line_completed_mutex_);
    line_completed_cv_.wait(ul, [this] { return line_completed_; });
    assert(buff_.empty() == false);
    return buff_.front();
  }

  int uflow() override {
    int result = underflow();
    buff_.erase(std::begin(buff_));
    if (result == '\n') {
      line_completed_ = false;
      line_completed_cv_.notify_one();
    }
    return result;
  }

  std::string buff_;
  bool line_completed_{false};
  std::condition_variable line_completed_cv_;
  std::mutex line_completed_mutex_;
} line_ostream_buf, line_istream_buf;

std::iostream line_ostream(&line_ostream_buf);
std::iostream line_istream(&line_istream_buf);


class UCIHandlerWrapper {
 public:
  bool sendCommandAndWaitForResponse(const std::string& command,
                                     const std::string& response,
                                     unsigned timeout_ms) {
    line_ostream.clear();
    std::thread t(&UCIHandlerWrapper::uciThread, this);
    t.detach();
    {
      std::unique_lock ul(uci_handler_started_mutex_);
      uci_handler_started_cv_.wait(ul, [this] { return uci_handler_started_ == true; });
    }
    auto start_time = std::chrono::steady_clock::now();
    unsigned time_elapsed = 0;
    line_istream << command << std::endl;
    bool response_found = false;
    do {
      while (line_ostream_buf.isLineReadyForRead() == true) {
        std::string s;
        std::getline(line_ostream, s);
        if (s.find(response) != std::string::npos) {
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
    } while (time_elapsed < timeout_ms);

    // Make all write command to ostream fail. It will quickly clear the buffer
    // so the other thread can unblock and read and handle quit command.
    line_ostream.setstate(std::ios_base::eofbit);

    line_istream << "quit" << std::endl;
    std::unique_lock ul(uci_handler_started_mutex_);
    uci_handler_started_cv_.wait(ul, [this] { return uci_handler_started_ == false; });

    return response_found;
  }

  void sendCommand(const std::string& command) {
    handler_.handleCommand(command);
  }

  const Board& getBoard() const {
    return handler_.getBoard();
  }
  
 private:
  void uciThread() {
    {
      std::unique_lock ul(uci_handler_started_mutex_);
      uci_handler_started_ = true;
    }
    uci_handler_started_cv_.notify_one();
    handler_.start();
    std::unique_lock ul(uci_handler_started_mutex_);
    uci_handler_started_ = false;
    uci_handler_started_cv_.notify_one();
  }

  UCIHandler handler_{line_istream, line_ostream};
  bool uci_handler_started_{false};
  std::mutex uci_handler_started_mutex_;
  std::condition_variable uci_handler_started_cv_;
};
    

TEST_PROCEDURE(UCIHandlerHandlesUnrecognizedCommand) {
  TEST_START
  UCIHandlerWrapper wrapper;
  VERIFY_TRUE(wrapper.sendCommandAndWaitForResponse("invalid_command", "Unrecognized command: invalid_command", 100));
  TEST_END
}

TEST_PROCEDURE(UCIHandlerHandlesCommandUCI) {
  TEST_START
  UCIHandlerWrapper wrapper;
  VERIFY_TRUE(wrapper.sendCommandAndWaitForResponse("uci", "id author ", 100));
  VERIFY_TRUE(wrapper.sendCommandAndWaitForResponse("uci", "id name ", 100));
  VERIFY_TRUE(wrapper.sendCommandAndWaitForResponse("uci", "uciok", 100));
  TEST_END
}

TEST_PROCEDURE(UCIHandlerHandlesCommandISREADY) {
  TEST_START
  UCIHandlerWrapper wrapper;
  VERIFY_TRUE(wrapper.sendCommandAndWaitForResponse("isready", "readyok", 100));
  TEST_END
}

TEST_PROCEDURE(UCIHandlerHandlesCommandGO) {
  TEST_START
  UCIHandlerWrapper wrapper;
  VERIFY_TRUE(wrapper.sendCommandAndWaitForResponse("go movetime 100", "info ", 120));
  VERIFY_TRUE(wrapper.sendCommandAndWaitForResponse("go movetime 500", "bestmove ", 550));
  VERIFY_FALSE(wrapper.sendCommandAndWaitForResponse("go movetime 500", "bestmove ", 499));
  TEST_END
}

TEST_PROCEDURE(UCIHandlerHandlesCommandSTOP) {
  TEST_START
  UCIHandlerWrapper wrapper;
  wrapper.sendCommand("go movetime 5000");
  VERIFY_TRUE(wrapper.sendCommandAndWaitForResponse("stop", "bestmove ", 100));
  TEST_END
}

TEST_PROCEDURE(UCIHandlerHandlesCommandPOSITION) {
  TEST_START
  UCIHandlerWrapper wrapper;
  const Board& board = wrapper.getBoard();
  wrapper.sendCommand("position fen 1nb1k1nr/pp1ppppp/1qp5/8/4P1Q1/1PN4N/P1PP1PPP/R1B1K2R b Kq - 7 15");
  VERIFY_STRINGS_EQUAL(board.createFEN().c_str(), "1nb1k1nr/pp1ppppp/1qp5/8/4P1Q1/1PN4N/P1PP1PPP/R1B1K2R b Kq - 7 15");
  wrapper.sendCommand("position startpos");
  VERIFY_STRINGS_EQUAL(board.createFEN().c_str(), "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  wrapper.sendCommand("position startpos moves d2d4 d7d5");
  VERIFY_STRINGS_EQUAL(board.createFEN().c_str(), "rnbqkbnr/ppp1pppp/8/3p4/3P4/8/PPP1PPPP/RNBQKBNR w KQkq d6 0 2");
  TEST_END
}

} // unnamed namespace

