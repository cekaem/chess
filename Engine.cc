#include "Engine.h"

#include <condition_variable>
#include <cstdlib>
#include <ctime>
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

  std::vector<Figure::Move> moves = board_.calculateMovesForFigures(color);
  for (Figure::Move& move: moves) {
    std::unique_lock<std::mutex> ul(number_of_threads_working_mutex_);
    number_of_threads_working_cv_.wait(
        ul, [this] { return number_of_threads_working_ < max_number_of_threads_; });
    std::thread t(&Engine::evaluateBoardMain, this, move);
    t.detach();
    ++number_of_threads_working_;
    debug_stream_ << SocketLog::lock << "Number of working threads: " << number_of_threads_working_ << SocketLog::endl;
  }

  // Wait for all threads to finish moves evaluation
  std::unique_lock<std::mutex> ul(number_of_threads_working_mutex_);
  number_of_threads_working_cv_.wait(ul, [this] { return number_of_threads_working_ == 0; });

  // Iterate through all moves and look for mate and for best value.
  // Also look for possible opponent's mate.
  auto border_values = findBorderValues(evaluated_moves_);
  int moves_to_mate = 0;
  int the_best_value = border_values.the_biggest_value;
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

  Figure::Move my_move;
  // Collect all best moves.
  std::vector<Figure::Move> the_best_moves;
  for (auto& move: evaluated_moves_) {
    if (moves_to_mate != 0) {
      if (move.moves_to_mate == moves_to_mate) {
        the_best_moves.push_back(move.move);
      }
    } else if (move.moves_to_mate == 0 && move.value == the_best_value) {
      the_best_moves.push_back(move.move);
    }
  }

  // Check which moves from the ones with the best value is the best in one move.
  std::vector<Figure::Move> the_best_direct_moves;
  int the_best_direct_move_value = -BorderValue;
  for (auto& move: the_best_moves) {
    if (moves_to_mate != 0) {
      the_best_direct_moves.push_back(move);
    } else {
      auto wrapper = board_.makeReversibleMove(move);

      std::vector<Figure::Move> all_moves;
      auto m = evaluateBoardForLastNode(board_, move, !color, false, all_moves);
      if (m.value > the_best_direct_move_value) {
        the_best_direct_moves.clear();
        the_best_direct_move_value = m.value;
      }
      if (m.value >= the_best_direct_move_value) {
        the_best_direct_moves.push_back(move);
      }
    }
  }
  BoardAssert(board_, the_best_direct_moves.size() > 0);
  // Choose randomly move from the best direct moves.
  my_move = the_best_direct_moves[generateRandomValue(the_best_direct_moves.size() - 1)];

  debug_stream_ << SocketLog::lock << "My move (" << moves_count_ << "): ";
  debug_stream_ << my_move.old_field << "-" << my_move.new_field << SocketLog::endl;
  board_.makeMove(my_move);
  ++moves_count_;
  evaluated_moves_.clear();
  return my_move;
}

Engine::Move Engine::evaluateBoardForLastNode(
    Board& board,
    const Figure::Move& current_move,
    Figure::Color color,
    bool my_move,
    std::vector<Figure::Move>& all_moves) const {
  std::vector<const Figure*> my_figures = my_move ? board.getFigures(color) : board.getFigures(!color);
  std::vector<const Figure*> enemy_figures = my_move ? board.getFigures(!color) : board.getFigures(color);
  int value = 0;
  for (const Figure* figure: my_figures) {
    value += figure->getValue();
  }
  for (const Figure* figure: enemy_figures) {
    value -= figure->getValue();
  }

  Board::GameStatus status = board.getGameStatus(color);
  int moves_to_mate = 0;
  if (status == Board::GameStatus::WHITE_WON || status == Board::GameStatus::BLACK_WON) {
    moves_to_mate = my_move ? -1 : 1;
    if (all_moves.empty() == false) {
      debug_stream_ << SocketLog::lock << "Found mate: " << all_moves << SocketLog::endl;
    }
  }

  return Move(current_move, value, moves_to_mate, status == Board::GameStatus::DRAW);
}

Engine::Move Engine::evaluateBoard(
    Board& board,
    const Figure::Move& current_move,
    Figure::Color color,
    bool my_move,
    int depths_remaining,
    std::vector<Figure::Move>& all_moves) const {
  Board::GameStatus status = board.getGameStatus(color);
  if (status != Board::GameStatus::NONE || depths_remaining == 0) {
    return evaluateBoardForLastNode(board, current_move, color, my_move, all_moves);
  }

  std::vector<Move> engine_moves;

  std::vector<Figure::Move> moves = board.calculateMovesForFigures(color);
  for (Figure::Move& move: moves) {
    auto wrapper = board.makeReversibleMove(move);
    all_moves.push_back(move);
    Engine::Move engine_move;
    engine_move = evaluateBoard(board, move, !color, !my_move, depths_remaining - 1, all_moves);
    all_moves.pop_back();
    engine_moves.push_back(engine_move);
  }

  int value = 0;
  auto border_values = findBorderValues(engine_moves);
  if (my_move == true) {
    value = border_values.the_biggest_value;
  } else {
    value = border_values.the_smallest_value;
  }
  int moves_to_mate = BorderValue;
  if (my_move == true) {
    if (border_values.the_smallest_positive_mate_value < BorderValue) {
      moves_to_mate = border_values.the_smallest_positive_mate_value + 1;
    } else if (border_values.zero_mate_value_exists == true) {
      moves_to_mate = 0;
    } else {
      BoardAssert(board, border_values.the_smallest_mate_value < 0);
      moves_to_mate = border_values.the_smallest_mate_value - 1;
    }
  } else {
    if (border_values.the_biggest_negative_mate_value > -BorderValue) {
      moves_to_mate = border_values.the_biggest_negative_mate_value - 1;
    } else if (border_values.zero_mate_value_exists == true) {
      moves_to_mate = 0;
    } else {
      BoardAssert(board, border_values.the_biggest_mate_value > 0);
      moves_to_mate = border_values.the_biggest_mate_value + 1;
    }
  }

  BoardAssert(board, (value != BorderValue && value != -BorderValue) || moves_to_mate != BorderValue);
  return Move(current_move, value, moves_to_mate, false);
}

void Engine::evaluateBoardMain(
    Figure::Move move) {
  std::vector<Figure::Move> all_moves;
  all_moves.push_back(move);
  Board copy = board_;
  Figure::Color color = copy.getFigure(move.old_field)->getColor();
  copy.makeMove(move);
  Move engine_move = evaluateBoard(copy, move, !color, false, search_depth_ - 1, all_moves);
  evaluated_moves_mutex_.lock();
  evaluated_moves_.push_back(engine_move);
  evaluated_moves_mutex_.unlock();
  number_of_threads_working_mutex_.lock();
  --number_of_threads_working_;
  debug_stream_ << SocketLog::lock << "Number of working threads: " << number_of_threads_working_ << SocketLog::endl;
  number_of_threads_working_mutex_.unlock();
  number_of_threads_working_cv_.notify_one();
}

void Engine::generateTree(Board& board, Figure::Color color, std::vector<Engine::Move>& moves) {
  for (auto& move: moves) {
    if (move.moves.empty() == false) {
      auto wrapper = board.makeReversibleMove(move.move);
      generateTree(board, !color, move.moves);
    } else {
      std::vector<Figure::Move> figure_moves = board.calculateMovesForFigures(color);
      for (Figure::Move& figure_move: figure_moves) {
        auto wrapper = board.makeReversibleMove(figure_move);
        Move new_move(figure_move);
        move.moves.push_back(new_move);
      }
    }
  }
}
