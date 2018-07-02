#include "Engine.h"

#include <cassert>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <stdexcept>
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
  std::vector<Figure::Move> all_moves;
  std::vector<const Figure*> figures = board_.getFigures(color);
  for (const Figure* figure: figures) {
    std::vector<Figure::Move> moves = figure->calculatePossibleMoves();
    all_moves.insert(all_moves.end(), moves.begin(), moves.end());
  }
  size_t moves_count = all_moves.size();
  debug_stream_ << "Moves count: " << moves_count << std::endl;

  auto my_move = all_moves[generateRandomValue(moves_count-1)];
  for (auto& move: all_moves) {
    if (move.is_mate == true) {
      my_move = move;
    }
  }

  debug_stream_ << "My move (" << moves_count_ << "): " << my_move.old_field << "-" << my_move.new_field << std::endl;
  board_.makeMove(my_move);
  ++moves_count_;
  return my_move;
}

std::pair<int, int> Engine::evaluatePosition(const Board& board, Figure::Color my_color) const {
  std::vector<const Figure*> my_figures = board.getFigures(my_color);
  std::vector<const Figure*> enemy_figures = board.getFigures(!my_color);
  int value = 0;
  for (const Figure* figure: my_figures) {
    value += figure->getValue();
  }
  for (const Figure* figure: enemy_figures) {
    value -= figure->getValue();
  }

  int moves_to_mate = 0;
  Board::GameStatus status = board.getGameStatus(my_color);
  if (my_color == Figure::WHITE && status == Board::GameStatus::WHITE_WON ||
      my_color == Figure::BLACK && status == Board::GameStatus::BLACK_WON) {
    moves_to_mate = 1;
  }
  if (my_color == Figure::WHITE && status == Board::GameStatus::BLACK_WON ||
      my_color == Figure::BLACK && status == Board::GameStatus::WHITE_WON) {
    moves_to_mate = -1;
  }

  return std::make_pair(value, moves_to_mate);
}

std::pair<int, int> Engine::evaluatePosition(
    const std::vector<Move>& moves,
    Figure::Color my_color,
    bool my_move) const {
  if (moves.empty() == true) {
    throw std::runtime_error("Internal engine's error: \"empty vector in Engine::evaluatePosition\"");
  }
  int value = 0;
  int moves_to_mate = 0;
  if (my_move) {
    value = -BorderValue;
    moves_to_mate = BorderValue;
  } else {
    value = BorderValue;
    moves_to_mate = -BorderValue;
  }

  std::vector<Move> helper;
  for (Move move: moves) {
    if (my_move == true && move.moves_to_mate <= moves_to_mate) {
      helper.push_back(move);
    }
    if (my_move == false && move.moves_to_mate >= moves_to_mate) {
      helper.push_back(move);
    }
  }
  if (moves.empty() == true) {
    throw std::runtime_error("Internal engine's error: \"empty vector(2) in Engine::evaluatePosition\"");
  }

  for (Move move: helper) {
    if (
  }
}
