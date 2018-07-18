#include "UCIHandler.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>

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
      std::string error_message("Unrecognized command: ");
      error_message.append(e.what());
      debug_stream_ << error_message << SocketLog::endl;
      ostr_ << error_message << std::endl;
    } catch (const EndProgramException&) {
      break;
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
  auto iter = std::find_if(std::begin(handlers_),
                           std::end(handlers_),
                           [cmd] (const auto& m) -> bool {
                             return cmd == m.first;
                           });
  if (iter == handlers_.end()) {
    throw UnknownCommandException(cmd);
  }
  debug_stream_ << "Recognized command: " << cmd << SocketLog::endl;

  std::vector<std::string> params;
  while (space_position != std::string::npos) {
    command = command.substr(space_position);
    ltrim(command);
    space_position = command.find(' ');
    std::string param;
    if (space_position == std::string::npos) {
      param = command;
    } else {
      param = command.substr(0, space_position);
    }
    if (param.empty() == false) {
      params.push_back(param);
    }
  }

  (this->*(iter->second))(params);
}

void UCIHandler::handleCommandUCI(const std::vector<std::string>& params) {
  ostr_ << "uciok" << std::endl;
}

void UCIHandler::handleCommandQuit(const std::vector<std::string>& params) {
  throw EndProgramException();
}
