#include "Engine.h"

#include <cassert>
#include <condition_variable>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <mutex>
#include <thread>
#include <utility>

#include "Board.h"
#include "Figure.h"

namespace {
static const int BorderValue = 1000;
}  // unnamed namespace

Engine::Engine(Board& board, unsigned search_depth, unsigned max_number_of_threads, std::ostream& debug_stream)
  : board_(board), search_depth_(search_depth), max_number_of_threads_(max_number_of_threads), debug_stream_(debug_stream) {
  srand(static_cast<unsigned int>(time(nullptr)));
}

int Engine::generateRandomValue(int max) const {
  return rand() % (max + 1);
}

Figure::Move Engine::makeMove(Figure::Color color) {
  Board::GameStatus status = board_.getGameStatus(color);
  if (status != Board::GameStatus::NONE) {
    throw Board::BadBoardStatusException(&board_);
  }

  Board copy = board_;
  std::vector<const Figure*> figures = board_.getFigures(color);
  for (const Figure* figure: figures) {
    std::vector<Figure::Move> moves = copy.calculateMovesForFigure(figure);
    for (Figure::Move& move: moves) {
      std::unique_lock<std::mutex> ul(number_of_threads_working_mutex_);
      number_of_threads_working_cv_.wait(
          ul, [this] { return number_of_threads_working_ < max_number_of_threads_; });
      std::thread t(&Engine::evaluateBoardMain, this, move, !color, false, search_depth_ - 1);
      t.detach();
      ++number_of_threads_working_;
    }
  }

  // Wait for all threads to finish moves evaluation
  std::unique_lock<std::mutex> ul(number_of_threads_working_mutex_);
  number_of_threads_working_cv_.wait(ul, [this] { return number_of_threads_working_ == 0; });

  // TODO: implement delaying certain mate
  // Iterate through all moves and look for mate and for best value.
  int moves_to_mate = BorderValue;
  int the_best_value = -BorderValue;
  Figure::Move mating_move;
  for (auto& move: evaluated_moves_) {
    if (move.second.moves_to_mate > 0 && move.second.moves_to_mate < moves_to_mate) {
      moves_to_mate = move.second.moves_to_mate;
      mating_move = move.first;
    }
    if (move.second.value > the_best_value) {
      the_best_value = move.second.value;
    }
  }

  Figure::Move my_move;
  if (moves_to_mate != BorderValue) {
    my_move = mating_move;
    debug_stream_ << "Found mate in " << (moves_to_mate / 2 + 1) << std::endl;
  } else {
    // Collect all moves with the best value.
    std::vector<Figure::Move> the_best_moves;
    for (auto& move: evaluated_moves_) {
      if (move.second.value == the_best_value) {
        the_best_moves.push_back(move.first);
      }
    }
    // Check which moves from the ones with the best value is the best in one move.
    std::vector<Figure::Move> the_best_direct_moves;
    int the_best_direct_move_value = -BorderValue;
    for (auto& move: the_best_moves) {
      auto wrapper = board_.makeReversibleMove(move);
      auto m = evaluateBoardForLastNode(board_, !color, false);
      if (m.value > the_best_direct_move_value) {
        the_best_direct_moves.clear();
        the_best_direct_move_value = m.value;
      }
      if (m.value >= the_best_direct_move_value) {
        the_best_direct_moves.push_back(move);
      }
    }
    // Choose randomly move from the best direct moves.
    my_move = the_best_direct_moves[generateRandomValue(the_best_direct_moves.size() - 1)];
  }

  debug_stream_ << "My move (" << moves_count_ << "): " << my_move.old_field << "-" << my_move.new_field << std::endl;
  board_.makeMove(my_move);
  ++moves_count_;
  evaluated_moves_.clear();
  return my_move;
}

Engine::Move Engine::evaluateBoardForLastNode(Board& board, Figure::Color color, bool my_move) const {
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
  }

  return Move(value, moves_to_mate, status == Board::GameStatus::DRAW);
}

Engine::Move Engine::evaluateBoard(
    Board& board,
    Figure::Color color,
    bool my_move,
    int depths_remaining) const {
  Board::GameStatus status = board.getGameStatus(color);
  if (status != Board::GameStatus::NONE || depths_remaining == 0) {
    return evaluateBoardForLastNode(board, color, my_move);
  }

  std::vector<Move> engine_moves;

  std::vector<const Figure*> figures = board.getFigures(color);
  for (const Figure* figure: figures) {
    std::vector<Figure::Move> moves = board.calculateMovesForFigure(figure);
    for (Figure::Move& move: moves) {
      auto wrapper = board.makeReversibleMove(move);
      Engine::Move engine_move;
      engine_move = evaluateBoard(board, !color, !my_move, depths_remaining - 1);
      engine_moves.push_back(engine_move);
    }
  }

  int value = 0;
  if (my_move) {
    value = -BorderValue;
  } else {
    value = BorderValue;
  }

  int the_smallest_value = 0;
  int the_smallest_positive_value = BorderValue;
  int the_biggest_value = 0;
  int the_biggest_negative_value = 0;
  bool zero_exists = false;

  for (auto& move: engine_moves) {
    if ((my_move == true && move.value >= value) ||
        (my_move == false && move.value <= value)) {
      value = move.value;
    }

    if (move.moves_to_mate == 0) {
      zero_exists = true;
      continue;
    }

    if (my_move == true) {
      if (move.moves_to_mate > 0 && move.moves_to_mate < the_smallest_positive_value) {
        the_smallest_positive_value = move.moves_to_mate;
      } else if (move.moves_to_mate < 0 && move.moves_to_mate < the_smallest_value) {
        the_smallest_value = move.moves_to_mate;
      }
    } else {
      if (move.moves_to_mate < 0 && move.moves_to_mate > the_biggest_negative_value) {
        the_biggest_negative_value = move.moves_to_mate;
      } else if (move.moves_to_mate > the_biggest_value) {
        the_biggest_value = move.moves_to_mate;
      }
    }
  }

  int moves_to_mate = BorderValue;
  if (my_move == true) {
    if (the_smallest_positive_value < BorderValue) {
      moves_to_mate = the_smallest_positive_value + 1;
    } else if (zero_exists == true) {
      moves_to_mate = 0;
    } else {
      assert(the_smallest_value < 0);
      moves_to_mate = the_smallest_value - 1;
    }
  } else {
    if (the_biggest_negative_value < 0) {
      moves_to_mate = the_biggest_negative_value;
    } else if (zero_exists == true) {
      moves_to_mate = 0;
    } else {
      assert(the_biggest_value > 0);
      moves_to_mate = the_biggest_value + 1;
    }
  }

  assert(value != BorderValue && value != -BorderValue && moves_to_mate != BorderValue);
  return Move(value, moves_to_mate, false);
}

void Engine::evaluateBoardMain(
    Figure::Move move,
    Figure::Color color,
    bool my_move,
    int depths_remaining) {
  Board copy = board_;
  copy.makeMove(move);
  Move engine_move = evaluateBoard(copy, color, my_move, depths_remaining);
  evaluated_moves_mutex_.lock();
  evaluated_moves_.push_back(std::make_pair(move, engine_move));
  evaluated_moves_mutex_.unlock();
  number_of_threads_working_mutex_.lock();
  --number_of_threads_working_;
  number_of_threads_working_mutex_.unlock();
  number_of_threads_working_cv_.notify_one();
}
