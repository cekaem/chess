#include "UCIHandler.h"

#include <algorithm>
#include <iostream>
#include <string>

using utils::SocketLog;

namespace {

constexpr int LoggerPort = 9090;

void ltrim(std::string& str) {
  str.erase(std::begin(str), std::find_if(std::begin(str), 
                                          std::end(str),
                                          [] (char c) -> bool { return std::isspace(c) == false; }));
}

}  // unnamed namespace

UCIHandler::UCIHandler(std::istream& istr, std::ostream& ostr, bool enable_logging)
  : istr_(istr), ostr_(ostr), engine_(board_, debug_stream_) {
  if (enable_logging == true) {
    ostr_ << "Waiting for logger to connect..." << std::endl;
    debug_stream_.waitForClient(LoggerPort);
    ostr_ << "Logger connected." << std::endl;
  }
}

void UCIHandler::start() {
  while(1) {
    try {
      std::string command;
      std::getline(istr_, command);
      handleCommand(command);
    } catch (const UnknownCommandException& e) {
      debug_stream_ << e.command << SocketLog::endl;
    }
  }
}

void UCIHandler::handleCommand(const std::string& line) {
  std::string command = line;
  debug_stream_ << "Received command: " << command << SocketLog::endl;
  ltrim(command);
  auto space_position = command.find(' ');
  std::string cmd;
  if (space_position == std::string::npos) {
    cmd = command;
  } else {
    cmd = command.substr(0, space_position);
  }
  debug_stream_ << "Recognized command: " << cmd << SocketLog::endl;
}
