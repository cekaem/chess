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
