#include "Engine.h"

#include <cassert>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <utility>

#include "Board.h"
#include "Figure.h"

namespace {
static const int BorderValue = 1000;
}  // unnamed namespace

Engine::Engine(Board& board, unsigned search_depth, std::ostream& debug_stream)
  : board_(board), search_depth_(search_depth), debug_stream_(debug_stream) {
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

  std::vector<std::pair<Figure::Move, Move>> moves_pairs;
  Board copy = board_;
  std::vector<const Figure*> figures = copy.getFigures(color);
  for (const Figure* figure: figures) {
    std::vector<Figure::Move> moves = copy.calculateMovesForFigure(figure);
    for (Figure::Move& move: moves) {
      auto wrapper = copy.makeReversibleMove(move);
      // debug_stream_ << "Evaluating move: " << move.old_field << "-" << move.new_field << std::endl;
      Engine::Move engine_move = evaluateBoard(copy, !color, false, search_depth_ - 1);
      moves_pairs.push_back(std::make_pair(move, engine_move));
    }
  }

  // TODO: implement delaying certain mate
  // Iterate through all moves and look for mate and for best value.
  int moves_to_mate = BorderValue;
  int the_best_value = -BorderValue;
  Figure::Move mating_move;
  for (auto& move: moves_pairs) {
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
  } else {
    // Collect all moves with the best value and choose one (random choice).
    std::vector<Figure::Move> the_best_moves;
    for (auto& move: moves_pairs) {
      if (move.second.value == the_best_value) {
        the_best_moves.push_back(move.first);
      }
    }
    my_move = the_best_moves[generateRandomValue(the_best_moves.size() - 1)];
  }

  debug_stream_ << "My move (" << moves_count_ << "): " << my_move.old_field << "-" << my_move.new_field << std::endl;
  board_.makeMove(my_move);
  ++moves_count_;
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
  if (status != Board::GameStatus::NONE) {
    return evaluateBoardForLastNode(board, color, my_move);
  }

  std::vector<Move> engine_moves;

  std::vector<const Figure*> figures = board.getFigures(color);
  for (const Figure* figure: figures) {
    std::vector<Figure::Move> moves = board.calculateMovesForFigure(figure);
    for (Figure::Move& move: moves) {
      // debug_stream_ << "Evaluating move: " << move.old_field << "-" << move.new_field << std::endl;
      auto wrapper = board.makeReversibleMove(move);
      Engine::Move engine_move;
      if (depths_remaining == 0) {
        engine_move = evaluateBoardForLastNode(board, !color, !my_move);
      } else {
        engine_move = evaluateBoard(board, !color, !my_move, depths_remaining - 1);
      }
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
  int the_smallest_positive_value = 0;
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
    if (the_smallest_positive_value > 0) {
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
