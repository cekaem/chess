#include "UCIHandler.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "Engine.h"
#include "Logger.h"
#include "utils/SocketLog.h"
#include "utils/Utils.h"


namespace {

const std::map<const char*,
               void(UCIHandler::*)(const std::vector<std::string>&)> g_handlers = {
  {"uci", &UCIHandler::handleCommandUCI},
  {"isready", &UCIHandler::handleCommandIsReady},
  {"position", &UCIHandler::handleCommandPosition},
  {"go", &UCIHandler::handleCommandGo},
  {"quit", &UCIHandler::handleCommandQuit}
};

}  // unnamed namespace


UCIHandler::UCIHandler(std::istream& istr, std::ostream& ostr)
  : istr_(istr), ostr_(ostr), engine_(board_) {
  board_.setStandardBoard();
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
  auto iter = std::find_if(std::begin(g_handlers),
                           std::end(g_handlers),
                           [cmd] (const auto& m) -> bool {
                             return cmd == m.first;
                           });
  if (iter == g_handlers.end()) {
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

void UCIHandler::handleCommandIsReady(const std::vector<std::string>& params) {
  ostr_ << "readyok" << std::endl;
}

void UCIHandler::handleCommandPosition(const std::vector<std::string>& params) {
  if (params.empty() == true) {
    LogWithEndLine(Logger::LogSection::UCI_HANDLER, "position: got no parameters");
    return;
  }
  if (params[0] == "fen") {
    if (params.size() < 7) {
      LogWithEndLine(Logger::LogSection::UCI_HANDLER, "position: got wrong fen string");
      return;
    }
    std::string fen;
    for (int i = 1; i < 7; ++i) {
      fen += params[i] + " ";
    }
    if (board_.setBoardFromFEN(fen) == true) {
      LogWithEndLine(Logger::LogSection::UCI_HANDLER, "position: set board from received fen");
    } else {
      LogWithEndLine(Logger::LogSection::UCI_HANDLER, "position: got wrong fen: ", fen);
    }
  } else if (params[0] == "starpos") {
    board_.setStandardBoard();
    LogWithEndLine(Logger::LogSection::UCI_HANDLER, "position: set starting position");
  }
}

void UCIHandler::handleCommandGo(const std::vector<std::string>& params) {
  if (params.empty() == true) {
    LogWithEndLine(Logger::LogSection::UCI_HANDLER, "go: got no parameters");
    return;
  }
  if (params[0] == "movetime") {
    if (params.size() < 2) {
      LogWithEndLine(Logger::LogSection::UCI_HANDLER, "go: got movetime without value");
      return;
    }
    unsigned time_to_move = 0;
    if (utils::str_2_uint(params[1], time_to_move) == false) {
      LogWithEndLine(Logger::LogSection::UCI_HANDLER, "go: got movetime with invalid value");
    }
    auto move = engine_.makeMove(time_to_move, 1000/* infinite depth */);
    std::stringstream response;
    response << "bestmove " << move.old_field << move.new_field;
    LogWithEndLine(Logger::LogSection::UCI_HANDLER, "go: sending response: ", response.str());
    ostr_ << response.str() << std::endl;
  }
}

void UCIHandler::handleCommandQuit(const std::vector<std::string>& params) {
  throw EndProgramException();
}
