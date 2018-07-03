#include "Engine.h"

#include <cassert>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <utility>

#include "Board.h"
#include "Figure.h"

Engine::Engine(Board& board, std::ostream& debug_stream)
  : board_(board), debug_stream_(debug_stream) {
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

  std::vector<Move> moves = generateTree(Board& board, Figure::Color color, SearchDepth - 1);
  std::pair<int, int> result = evaluateMoves(moves, color, true);
  int moves_to_mate = BorderValue;
  int move_value = -BorderValue;
  for (const auto& r: result) {
    if 
  }
  if (!move_calculated) {
    my_move = all_moves[generateRandomValue(moves_count - 1)];
  }

  debug_stream_ << "My move (" << moves_count_ << "): " << my_move.old_field << "-" << my_move.new_field << std::endl;
  board_.makeMove(my_move);
  ++moves_count_;
  return my_move;
}

std::pair<int, int> Engine::evaluateMoves(const Board& board, Figure::Color my_color) const {
  std::vector<const Figure*> my_figures = board.getFigures(my_color);
  std::vector<const Figure*> enemy_figures = board.getFigures(!my_color);
  int value = 0;
  for (const Figure* figure: my_figures) {
    value += figure->getValue();
  }
  for (const Figure* figure: enemy_figures) {
    value -= figure->getValue();
  }

  Board::GameStatus status = board.getGameStatus(my_color);
  assert(my_color == Figure::WHITE ?
      status != Board::GameStatus::BLACK_WON : status != Board::GameStatus::WHITE_WON);
  int moves_to_mate =
      (status == Board::GameStatus::WHITE_WON || status == Board::GameStatus::BLACK_WON) ? 0 : 1;

  return std::make_pair(value, moves_to_mate);
}

std::pair<int, int> Engine::evaluateMoves(
    const std::vector<Move>& moves,
    Figure::Color my_color,
    bool my_move) const {
  static const int BorderValue = 100;

  assert(moves.empty() == false);

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

  for (auto& move: moves) {
    if ((my_move == true && move.value >= value) ||
        (my_move == false && move.value <= value)) {
      value = move.value;
    }

    if (move.moves_to_mate == 0) {
      zero_exists == true;
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
  return std::make_pair(value, moves_to_mate);
}

std::vector<Engine::Move> Engine::generateTree(Board& board, Figure::Color my_color, int depths_remaining) {
  std::vector<Engine::Move> result;
  std::vector<const Figure*> my_figures = board_.getFigures(my_color);
  for (const Figure* figure: my_figures) {
    std::vector<Figure::Move> moves = figure->calculatePossibleMoves();
    for (Figure::Move& move: moves) {

    }
  }
}
