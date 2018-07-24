#include "Engine.h"

#include <condition_variable>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>

#include "Board.h"
#include "Figure.h"

using utils::SocketLog;

Engine::Engine(
    Board& board,
    unsigned search_depth,
    unsigned max_number_of_threads,
    SocketLog& debug_stream)
  : Engine(board, debug_stream) {
  search_depth_ = search_depth;
  max_number_of_threads_ = max_number_of_threads;
}

Engine::Engine(Board& board, SocketLog& debug_stream)
  : board_(board), debug_stream_(debug_stream) {
  srand(static_cast<unsigned int>(time(nullptr)));
}

Engine::~Engine() {
  debug_stream_ << SocketLog::endLogging;
}

int Engine::generateRandomValue(int max) const {
  return rand() % (max + 1);
}

Engine::BorderValues Engine::findBorderValues(const std::vector<Engine::Move>& moves) const {
  BorderValues result;

  for (auto& move: moves) {
    if (move.moves_to_mate == 0 && move.value > result.the_biggest_value) {
      result.the_biggest_value = move.value;
    }
    if (move.moves_to_mate == 0 && move.value < result.the_smallest_value) {
      result.the_smallest_value = move.value;
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

Figure::Move Engine::makeMove() {
  Figure::Color color = board_.getSideToMove();
  Board::GameStatus status = board_.getGameStatus(color);
  if (status != Board::GameStatus::NONE) {
    throw Board::BadBoardStatusException(&board_);
  }

  std::vector<Move> moves;
  auto figures_moves = board_.calculateMovesForFigures(color);
  for (auto& move: figures_moves) {
    moves.push_back(Move(move, nullptr));
  }
  for (unsigned depth = 1; depth < search_depth_; ++depth) {
    debug_stream_ << SocketLog::lock << "Starting calculating depth " << depth + 1 << SocketLog::endl;
    for (Move& move: moves) {
      std::unique_lock<std::mutex> ul(number_of_threads_working_mutex_);
      number_of_threads_working_cv_.wait(
          ul, [this] { return number_of_threads_working_ < max_number_of_threads_; });
      std::thread t(&Engine::generateTreeMain, this, std::ref(move));
      t.detach();
      ++number_of_threads_working_;
      debug_stream_ << SocketLog::lock << "Number of working threads: " << number_of_threads_working_ << SocketLog::endl;
    }
    debug_stream_ << SocketLog::lock << "Finished calculating depth " << depth + 1 << SocketLog::endl;
  }

  // Wait for all threads to finish moves evaluation
  std::unique_lock<std::mutex> ul(number_of_threads_working_mutex_);
  number_of_threads_working_cv_.wait(ul, [this] { return number_of_threads_working_ == 0; });

  // Iterate through all moves and look for mate and for best value.
  // Also look for possible opponent's mate.
  auto border_values = findBorderValues(moves);
  int moves_to_mate = 0;
  int the_best_value =
      color == Figure::WHITE ?
      border_values.the_biggest_value :
      border_values.the_smallest_value;
  if (border_values.the_smallest_positive_mate_value != BorderValue) {
    moves_to_mate = border_values.the_smallest_positive_mate_value;
    debug_stream_ << SocketLog::lock << "Found mate in " << (moves_to_mate / 2 + 1) << SocketLog::endl;
  } else if (border_values.zero_mate_value_exists == true) {
    moves_to_mate = 0;
  } else {
    moves_to_mate = border_values.the_smallest_mate_value;
    debug_stream_ << SocketLog::lock << "Found opponent's mate in " << -(moves_to_mate / 2 - 1) << SocketLog::endl;
  }
  BoardAssert(board_, moves_to_mate != BorderValue);

  // Collect all best moves.
  std::vector<Move> the_best_moves;
  for (auto& move: moves) {
    if (moves_to_mate != 0) {
      if (move.moves_to_mate == moves_to_mate) {
        the_best_moves.push_back(move);
      }
    } else if (move.moves_to_mate == 0 && move.value == the_best_value) {
      the_best_moves.push_back(move);
    }
  }

  // Check which moves from the ones with the best value is the best in one move.
  std::vector<Move> the_best_direct_moves;
  int the_best_direct_move_value = -BorderValue;
  for (auto& move: the_best_moves) {
    if (moves_to_mate != 0) {
      the_best_direct_moves.push_back(move);
    } else {
      evaluateBoardForLastNode(board_, move);
      if (move.value > the_best_direct_move_value) {
        the_best_direct_moves.clear();
        the_best_direct_move_value = move.value;
      }
      if (move.value >= the_best_direct_move_value) {
        the_best_direct_moves.push_back(move);
      }
    }
  }
  BoardAssert(board_, the_best_direct_moves.size() > 0);
  // Choose randomly move from the best direct moves.
  Move my_move = the_best_direct_moves[generateRandomValue(the_best_direct_moves.size() - 1)];

  debug_stream_ << SocketLog::lock << "My move (" << moves_count_ << "): ";
  debug_stream_ << my_move.move.old_field << "-" << my_move.move.new_field << SocketLog::endl;
  board_.makeMove(my_move.move);
  ++moves_count_;
  return my_move.move;
}

void Engine::evaluateBoardForLastNode(
    Board& board, Engine::Move& current_move) const {
  auto wrapper = board.makeReversibleMove(current_move.move);
  std::vector<const Figure*> white_figures = board.getFigures(Figure::WHITE);
  std::vector<const Figure*> black_figures = board.getFigures(Figure::BLACK);
  current_move.value = 0;
  for (const Figure* figure: white_figures) {
    current_move.value += figure->getValue();
  }
  for (const Figure* figure: black_figures) {
    current_move.value -= figure->getValue();
  }

  current_move.moves_to_mate = 0;
  if (board.isKingCheckmated(Figure::WHITE) == true) {
    current_move.moves_to_mate = -1;
  }
  if (board.isKingCheckmated(Figure::BLACK) == true) {
    current_move.moves_to_mate = 1;
  }
  if (current_move.moves_to_mate != 0) {
    debug_stream_ << SocketLog::lock << "Found mate: ";
    std::function<void(const Engine::Move&)> lambda;
    lambda = [this, &lambda](const Engine::Move& move) -> void {
      if (move.parent != nullptr) {
        lambda(*move.parent);
      }
      debug_stream_ << move.move << " ";
    };
    lambda(current_move);
    debug_stream_ << SocketLog::endl;
  }
}

void Engine::evaluateBoard(Board& board, Engine::Move& current_move) const {
  if (current_move.moves.empty() == true) {
    evaluateBoardForLastNode(board, current_move);
    return;
  }

  current_move.value = 0;
  auto border_values = findBorderValues(current_move.moves);
  Figure::Color color = board.getFigure(current_move.move.old_field)->getColor();
  if (color == Figure::WHITE) {
    current_move.value = border_values.the_biggest_value;
  } else {
    current_move.value = border_values.the_smallest_value;
  }
  current_move.moves_to_mate = BorderValue;
  if (color == Figure::WHITE) {
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

  BoardAssert(board, (current_move.value != BorderValue && current_move.value != -BorderValue) ||
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
    for (auto& m: move.moves) {
      generateTree(board, !color, m);
    }
  } else {
    Board::GameStatus status = board.getGameStatus(color);
    if (status == Board::GameStatus::NONE) {
      auto wrapper = board.makeReversibleMove(move.move);
      std::vector<Figure::Move> figure_moves = board.calculateMovesForFigures(!color);
      for (Figure::Move& figure_move: figure_moves) {
        Move new_move(figure_move, &move);
        move.moves.push_back(new_move);
      }
    }
  }
  evaluateBoard(board, move);
}

void Engine::onThreadFinished() {
  number_of_threads_working_mutex_.lock();
  --number_of_threads_working_;
  debug_stream_ << SocketLog::lock << "Number of working threads: " << number_of_threads_working_ << SocketLog::endl;
  number_of_threads_working_mutex_.unlock();
  number_of_threads_working_cv_.notify_one();
}
