#ifndef UCI_HANDLER_H
#define UCI_HANDLER_H

#include <exception>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "Board.h"
#include "Engine.h"
#include "utils/SocketLog.h"


class UCIHandler {
 public:
  struct UnknownCommandException : public std::exception {
    UnknownCommandException(const std::string& cmd) : command(cmd) {}
    const char* what() const noexcept override { return command.c_str(); }
    const std::string command;
  };

  struct EndProgramException : public std::exception {};

  UCIHandler(std::istream& istr, std::ostream& ostr, bool enable_logging = false);
  void start();

 private:
  const std::map<const char*,
                 void(UCIHandler::*)(const std::vector<std::string>&)> handlers_ = {
    {"uci", &UCIHandler::handleCommandUCI},
    {"quit", &UCIHandler::handleCommandQuit}
  };

  void handleCommand(const std::string& command);
  void handleCommandUCI(const std::vector<std::string>& params);
  void handleCommandQuit(const std::vector<std::string>& params);

  std::istream& istr_;
  std::ostream& ostr_;
  Board board_;
  utils::SocketLog debug_stream_;
  Engine engine_;
};

#endif  // UCI_HANDLER_H
