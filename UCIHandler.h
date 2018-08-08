#ifndef UCI_HANDLER_H
#define UCI_HANDLER_H

#include <condition_variable>
#include <exception>
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "Board.h"
#include "Engine.h"


class UCIHandler {
 public:
  struct UnknownCommandException : public std::exception {
    UnknownCommandException(const std::string& cmd) : command(cmd) {}
    const char* what() const noexcept override { return command.c_str(); }
    const std::string command;
  };

  struct HandleOfCommandFailed : public std::exception {
    HandleOfCommandFailed(const std::string& cmd) : command(cmd) {}
    const char* what() const noexcept override { return command.c_str(); }
    const std::string command;
  };

  struct EndProgramException : public std::exception {};

  UCIHandler(std::istream& istr, std::ostream& ostr);
  void start();
  const Board& getBoard() const { return board_; }

  // public for testing purposes
  void handleCommand(const std::string& command);

  bool handleCommandUCI(const std::vector<std::string>& params);
  bool handleCommandQuit(const std::vector<std::string>& params);
  bool handleCommandIsReady(const std::vector<std::string>& params);
  bool handleCommandPosition(const std::vector<std::string>& params);
  bool handleCommandGo(const std::vector<std::string>& params);
  bool handleCommandStop(const std::vector<std::string>& params);

 private:
  void calculateMoveOnAnotherThread(unsigned time_for_move, unsigned max_depth);
  void sendInfoToGUI(Engine::SearchInfo info) const;

  bool move_calculation_in_progress_{false};
  std::mutex move_calculation_in_progress_mutex_;
  std::condition_variable move_calculation_in_progress_cv_;
  std::istream& istr_;
  std::ostream& ostr_;
  Board board_;
  Engine engine_;
};

#endif  // UCI_HANDLER_H
