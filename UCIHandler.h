#ifndef UCI_HANDLER_H
#define UCI_HANDLER_H

#include <exception>
#include <iostream>
#include <string>

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

  UCIHandler(std::istream& istr, std::ostream& ostr, bool enable_logging = false);
  void start();

 private:
  void handleCommand(const std::string& command);

  std::istream& istr_;
  std::ostream& ostr_;
  Board board_;
  utils::SocketLog debug_stream_;
  Engine engine_;
};

#endif  // UCI_HANDLER_H
