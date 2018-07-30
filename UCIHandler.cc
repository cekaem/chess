#include "UCIHandler.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "Engine.h"
#include "Logger.h"
#include "utils/Utils.h"


UCIHandler::UCIHandler(std::istream& istr, std::ostream& ostr)
  : istr_(istr), ostr_(ostr), engine_(board_) {
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
      LogWithEndLine(Logger::LogSection::UCI_HANDLER, error_message);
      ostr_ << error_message << std::endl;
    } catch (const EndProgramException&) {
      break;
    }
  }
}

void UCIHandler::handleCommand(const std::string& line) {
  std::string command = line;
  LogWithEndLine(Logger::LogSection::UCI_HANDLER, "Received command: ", command);
  utils::ltrim(command);
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
  LogWithEndLine(Logger::LogSection::UCI_HANDLER, "Recognized command: ", cmd);

  std::vector<std::string> params;
  while (space_position != std::string::npos) {
    command = command.substr(space_position);
    utils::ltrim(command);
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
