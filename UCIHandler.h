#ifndef UCI_HANDLER_H
#define UCI_HANDLER_H

#include <exception>
#include <iostream>
#include <map>
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

  struct EndProgramException : public std::exception {};

  UCIHandler(std::istream& istr, std::ostream& ostr);
  void start();

  void handleCommandUCI(const std::vector<std::string>& params);
  void handleCommandQuit(const std::vector<std::string>& params);
  void handleCommandIsReady(const std::vector<std::string>& params);
  void handleCommandPosition(const std::vector<std::string>& params);
  void handleCommandGo(const std::vector<std::string>& params);
  void handleCommandStop(const std::vector<std::string>& params);

 private:
  void handleCommand(const std::string& command);
  void calculateMoveOnAnotherThread(unsigned time_for_move, unsigned max_depth);

  bool move_calculation_in_progress_{false};
  std::istream& istr_;
  std::ostream& ostr_;
  Board board_;
  Engine engine_;
};

#endif  // UCI_HANDLER_H
