#include "Engine.h"

#include <condition_variable>
#include <cstdlib>
#include <chrono>
#include <ctime>
#include <functional>
#include <iostream>
#include <mutex>
#include <stdio.h>
#include <thread>
#include <unistd.h>

#include "Board.h"
#include "Figure.h"
#include "Logger.h"
#include "utils/SocketLog.h"
#include "utils/Utils.h"

using utils::SocketLog;


namespace {

// Move modificators
constexpr int CastlingModificator = 50;
constexpr int CheckModificator = 10;
constexpr int MoveForeclosingCastlingModificator = -50;

// Position modificators
constexpr int KnightAtTheBorderModificator = -25;

};


Engine::Engine(
    Board& board,
    unsigned max_number_of_threads)
  : Engine(board) {
  max_number_of_threads_ = max_number_of_threads;
}

Engine::Engine(Board& board) : board_(board) {
  srand(static_cast<unsigned int>(clock()));
  Logger::getLogger().alertOnMemoryConsumption(
      max_memory_consumption_,
      std::bind(&Engine::onMaxMemoryConsumptionExceeded, this, std::placeholders::_1));
}

int Engine::generateRandomValue(int max) const {
  return rand() % (max + 1);
}

Engine::BorderValues Engine::findBorderValues(const std::vector<Engine::Move>& moves) const {
  BorderValues result;

  for (auto& move: moves) {
    if (move.moves_to_mate == 0 && move.value_cp > result.the_biggest_value) {
      result.the_biggest_value = move.value_cp;
    }
    if (move.moves_to_mate == 0 && move.value_cp < result.the_smallest_value) {
      result.the_smallest_value = move.value_cp;
    }
    if (move.moves_to_mate == 0) {
      result.zero_mate_value_exists = true;
      continue;
    }
    if (move.moves_to_mate > 0 && move.moves_to_mate < result.the_smallest_positive_mate_value) {
      result.the_smallest_positive_mate_value = move.moves_to_mate;
    }
    if (move.moves_to_mate < result.the_smallest_mate_value) {
      result.the_smallest_mate_value = move.moves_to_mate;
    }
    if (move.moves_to_mate < 0 && move.moves_to_mate > result.the_biggest_negative_mate_value) {
      result.the_biggest_negative_mate_value = move.moves_to_mate;
    }
    if (move.moves_to_mate > result.the_biggest_mate_value) {
      result.the_biggest_mate_value = move.moves_to_mate;
    }
  }
  return result;
}

void Engine::onTimerExpired() {
  LogWithEndLine(Logger::LogSection::ENGINE_TIMER, "Timer expired");
  end_calculations_ = true;
}

void Engine::onMaxMemoryConsumptionExceeded(unsigned memory_consumption) {
  end_calculations_ = true;
}

Figure::Move Engine::makeMove(unsigned time_for_move, unsigned search_depth) {
  auto info = startSearch(time_for_move, search_depth);
  auto move = info.best_line[0];
  ++moves_count_;
  board_.makeMove(move);
  return move;
}


Engine::SearchInfo Engine::startSearch(unsigned time_for_move, unsigned search_depth) {
  end_calculations_ = false;
  auto start_time = std::chrono::steady_clock::now();
  if (time_for_move > 0) {
    LogWithEndLine(Logger::LogSection::ENGINE_TIMER, "Starting timer");
    timer_.start(time_for_move, std::bind(&Engine::onTimerExpired, this));
  }
  Figure::Color color = board_.getSideToMove();
  Board::GameStatus status = board_.getGameStatus(color);
  if (status != Board::GameStatus::NONE) {
    throw Board::BadBoardStatusException(&board_);
  }

  std::vector<Move> moves;
  auto figures_moves = board_.calculateMovesForFigures(color);
  nodes_evaluated_ = figures_moves.size();
  for (auto& move: figures_moves) {
    moves.push_back(Move(move, nullptr));
  }
  for (unsigned depth = 1; depth < search_depth && end_calculations_ == false; ++depth) {
    nodes_evaluated_ = 0u;
    LogWithEndLine(Logger::LogSection::ENGINE_MOVE_SEARCHES, "Starting calculating depth ", depth + 1);
    for (Move& move: moves) {
      std::unique_lock<std::mutex> ul(number_of_threads_working_mutex_);
      number_of_threads_working_cv_.wait(
          ul, [this] { return number_of_threads_working_ < max_number_of_threads_; });
      std::thread t(&Engine::generateTreeMain, this, std::ref(move));
      t.detach();
      ++number_of_threads_working_;
      LogWithEndLine(Logger::LogSection::ENGINE_THREADS, "Number of working threads: ", number_of_threads_working_);
    }
    // Wait for all threads to finish moves evaluation
    std::unique_lock<std::mutex> ul(number_of_threads_working_mutex_);
    number_of_threads_working_cv_.wait(ul, [this] { return number_of_threads_working_ == 0; });
    LogWithEndLine(Logger::LogSection::ENGINE_MOVE_SEARCHES, "Finished calculating depth ", depth + 1);
  }
  timer_.stop();

  SearchInfo info;
  info.nodes = nodes_evaluated_;
  std::function<void(std::vector<Move>&, Figure::Color)> lambda;
  lambda = [this, &info, &lambda](std::vector<Move>& moves, Figure::Color color) -> void {
    ++info.depth;
    Move* the_best_move = lookForTheBestMove(moves, color);
    info.best_line.push_back(the_best_move->move);
    if (info.depth == 1) {
      info.score_cp = the_best_move->value_cp;
      info.score_mate = the_best_move->moves_to_mate / 2;
    }
    if (the_best_move->moves.empty() == false) {
      auto wrapper = board_.makeReversibleMove(the_best_move->move);
      lambda(the_best_move->moves, !color);
    }
  };
  lambda(moves, color);

  if (color == Figure::BLACK) {
    info.score_cp = -info.score_cp;
    info.score_mate = -info.score_mate;
  }

  if (info.score_mate != 0) {
    LogWithEndLine(Logger::LogSection::ENGINE_MATES, "=== Found mate in ", info.score_mate, " ===");
  }

  auto end_time = std::chrono::steady_clock::now();
  auto time_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
  Log(Logger::LogSection::ENGINE_MOVE_SEARCHES, SocketLog::lock);
  Log(Logger::LogSection::ENGINE_MOVE_SEARCHES, "Best line (", info.score_cp, ", ");
  Log(Logger::LogSection::ENGINE_MOVE_SEARCHES, info.score_mate, "): ");
  Log(Logger::LogSection::ENGINE_MOVE_SEARCHES,  info.best_line);
  Log(Logger::LogSection::ENGINE_MOVE_SEARCHES, " (calculation time: ", time_elapsed, ")");
  Log(Logger::LogSection::ENGINE_MOVE_SEARCHES, SocketLog::endl);
  info.time = time_elapsed;

  return info;
}

std::pair<int, int> Engine::evaluateBorderValues(Engine::BorderValues values, Figure::Color color) const {
  int moves_to_mate = 0;
  int the_best_value = color == Figure::WHITE ? values.the_biggest_value : values.the_smallest_value;
  if (color == Figure::WHITE) {
    if (values.the_smallest_positive_mate_value != BorderValue) {
      moves_to_mate = values.the_smallest_positive_mate_value;
    } else if (values.zero_mate_value_exists == true) {
      moves_to_mate = 0;
    } else {
      moves_to_mate = values.the_smallest_mate_value;
    }
  } else {
    if (values.the_biggest_negative_mate_value != -BorderValue) {
      moves_to_mate = values.the_biggest_negative_mate_value;
    } else if (values.zero_mate_value_exists == true) {
      moves_to_mate = 0;
    } else {
      moves_to_mate = values.the_biggest_mate_value;
    }
  }
  BoardAssert(board_, moves_to_mate != BorderValue && moves_to_mate != -BorderValue);
  return std::make_pair(the_best_value, moves_to_mate);
}

Engine::Move* Engine::lookForTheBestMove(std::vector<Engine::Move>& moves, Figure::Color color) const {
  // Iterate through all moves and look for mate and for best value.
  // Also look for possible opponent's mate.
  auto border_values = findBorderValues(moves);
  std::pair<int, int> evaluation = evaluateBorderValues(border_values, color);
  int the_best_value = evaluation.first;
  int moves_to_mate = evaluation.second;

  // Collect all best moves.
  std::vector<Move*> the_best_moves;
  for (auto& move: moves) {
    if (moves_to_mate != 0) {
      if (move.moves_to_mate == moves_to_mate) {
        the_best_moves.push_back(&move);
      }
    } else if (move.moves_to_mate == 0 && move.value_cp == the_best_value) {
      the_best_moves.push_back(&move);
    }
  }

  // Check which moves from the ones with the best value is the best in one move.
  std::vector<Move*> the_best_direct_moves;
  int the_best_direct_move_value = color == Figure::WHITE ? -BorderValue : BorderValue;
  for (auto* move: the_best_moves) {
    if (moves_to_mate != 0) {
      the_best_direct_moves.push_back(move);
    } else {
      evaluateBoardForLastNode(board_, *move);
      if ((color == Figure::WHITE && move->value_cp > the_best_direct_move_value) ||
          (color == Figure::BLACK && move->value_cp < the_best_direct_move_value)) {
        the_best_direct_moves.clear();
        the_best_direct_move_value = move->value_cp;
      }
      if ((color == Figure::WHITE && move->value_cp >= the_best_direct_move_value) ||
          (color == Figure::BLACK && move->value_cp <= the_best_direct_move_value)) {
        the_best_direct_moves.push_back(move);
      }
    }
  }
  BoardAssert(board_, the_best_direct_moves.size() > 0);
  // Choose randomly move from the best direct moves.
  Move* my_move = the_best_direct_moves[generateRandomValue(the_best_direct_moves.size() - 1)];
  return my_move;
}

int Engine::calculateMoveModificator(Board& board, const Move& move) const {
  int result = 0;
  if (move.move.castling != Figure::Move::Castling::LAST) {
    result += CastlingModificator;
  }
  if (move.move.is_check == true) {
    result += CheckModificator;
  }
  const Figure* figure = board.getFigure(move.move.old_field);
  Figure::Color color = figure->getColor();
  if (move.move.castling == Figure::Move::Castling::LAST &&
      board.canKingCastle(color) == true) {
    auto wrapper = board.makeReversibleMove(move.move);
    if (board.canKingCastle(color) == false) {
      result += MoveForeclosingCastlingModificator;
    }
  }

  return result;
}

int Engine::calculatePositionValue(const Board& board) const {
  int result = 0;
  const auto& figures = board.getFigures();
  for (const auto& figure: figures) {
    Figure::Color color = figure->getColor();
    Field position = figure->getPosition();
    result += color == Figure::WHITE ? figure->getValue() : -figure->getValue();
    if (figure->getType() == Figure::KNIGHT &&
        (position.letter == Field::A || position.letter == Field::H)) {
      result += color == Figure::WHITE ? KnightAtTheBorderModificator : -KnightAtTheBorderModificator;
    }
  }
  return result;
}

void Engine::evaluateBoardForLastNode(
    Board& board, Engine::Move& current_move) const {
  Figure::Color color = board.getFigure(current_move.move.old_field)->getColor();
  int move_modificator = calculateMoveModificator(board, current_move);
  auto wrapper = board.makeReversibleMove(current_move.move);
  current_move.value_cp = calculatePositionValue(board);
  if (color == Figure::WHITE) {
    current_move.value_cp += move_modificator;
  } else {
    current_move.value_cp -= move_modificator;
  }

  current_move.moves_to_mate = 0;
  if (board.isKingCheckmated(Figure::WHITE) == true) {
    current_move.moves_to_mate = -1;
  }
  if (board.isKingCheckmated(Figure::BLACK) == true) {
    current_move.moves_to_mate = 1;
  }
  if (current_move.moves_to_mate != 0 && Logger::shouldLog(Logger::LogSection::ENGINE_MATES)) {
    Log(Logger::LogSection::ENGINE_MATES, SocketLog::lock, "Found mate: ");
    std::function<void(const Engine::Move&)> lambda;
    lambda = [this, &lambda](const Engine::Move& move) -> void {
      if (move.parent != nullptr) {
        lambda(*move.parent);
      }
      Log(Logger::LogSection::ENGINE_MATES, move.move, " ");
    };
    lambda(current_move);
    Log(Logger::LogSection::ENGINE_MATES, SocketLog::endl);
  }
}

void Engine::evaluateBoard(Board& board, Engine::Move& current_move) const {
  if (current_move.moves.empty() == true) {
    evaluateBoardForLastNode(board, current_move);
    return;
  }

  current_move.value_cp = 0;
  int move_modificator = calculateMoveModificator(board, current_move);
  auto border_values = findBorderValues(current_move.moves);
  Figure::Color color = board.getFigure(current_move.move.old_field)->getColor();
  if (color == Figure::WHITE) {
    current_move.value_cp = border_values.the_biggest_value;
    current_move.value_cp += move_modificator;
  } else {
    current_move.value_cp = border_values.the_smallest_value;
    current_move.value_cp -= move_modificator;
  }
  current_move.moves_to_mate = BorderValue;
  if (color == Figure::BLACK) {
    if (border_values.the_smallest_positive_mate_value < BorderValue) {
      current_move.moves_to_mate = border_values.the_smallest_positive_mate_value + 1;
    } else if (border_values.zero_mate_value_exists == true) {
      current_move.moves_to_mate = 0;
    } else {
      BoardAssert(board, border_values.the_smallest_mate_value < 0);
      current_move.moves_to_mate = border_values.the_smallest_mate_value - 1;
    }
  } else {
    if (border_values.the_biggest_negative_mate_value > -BorderValue) {
      current_move.moves_to_mate = border_values.the_biggest_negative_mate_value - 1;
    } else if (border_values.zero_mate_value_exists == true) {
      current_move.moves_to_mate = 0;
    } else {
      BoardAssert(board, border_values.the_biggest_mate_value > 0);
      current_move.moves_to_mate = border_values.the_biggest_mate_value + 1;
    }
  }

  BoardAssert(board, (current_move.value_cp != BorderValue && current_move.value_cp != -BorderValue) ||
                     current_move.moves_to_mate != BorderValue);
}

void Engine::generateTreeMain(Engine::Move& move) {
  Board copy = board_;
  Figure::Color color = copy.getFigure(move.move.old_field)->getColor();
  generateTree(copy, color, move);
  onThreadFinished();
}

void Engine::generateTree(Board& board, Figure::Color color, Engine::Move& move) {
  if (move.moves.empty() == false) {
    auto wrapper = board.makeReversibleMove(move.move);
    nodes_evaluated_ += move.moves.size();
    for (auto& m: move.moves) {
      if (end_calculations_ == true) {
        break;
      }
      generateTree(board, !color, m);
    }
  } else {
    Board::GameStatus status = board.getGameStatus(color);
    if (status == Board::GameStatus::NONE) {
      auto wrapper = board.makeReversibleMove(move.move);
      std::vector<Figure::Move> figures_moves = board.calculateMovesForFigures(!color);
      nodes_evaluated_ += figures_moves.size();
      for (Figure::Move& figure_move: figures_moves) {
        Move new_move(figure_move, &move);
        evaluateBoardForLastNode(board, new_move);
        move.moves.push_back(new_move);
      }
    }
  }
  evaluateBoard(board, move);
}

void Engine::onThreadFinished() {
  number_of_threads_working_mutex_.lock();
  --number_of_threads_working_;
  LogWithEndLine(Logger::LogSection::ENGINE_THREADS, "Number of working threads: ", number_of_threads_working_);
  number_of_threads_working_mutex_.unlock();
  number_of_threads_working_cv_.notify_one();
}
