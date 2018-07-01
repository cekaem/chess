#include "Engine.h"

#include <cassert>
#include <cstdlib>
#include <ctime>
#include <iostream>

#include "Board.h"
#include "Figure.h"

Engine::Engine(Board& board, std::ostream& debug_stream)
  : board_(board), debug_stream_(debug_stream) {
  srand(static_cast<unsigned int>(time(nullptr)));
}

Engine::Status Engine::getStatus() const {
  Engine::Status status = isCheckMate();
  if (status == Engine::Status::WHITE_WON ||
      status == Engine::Status::BLACK_WON) {
    return status;
  }
  if (isStaleMate() || isDraw()) {
    return Status::DRAW;
  }
  return Status::NONE;
}

Engine::Status Engine::isCheckMate() const {
  const King* king = board_.getKing(Figure::WHITE);
  if (king->isCheckmated()) {
    return Status::BLACK_WON;
  }
  king = board_.getKing(Figure::BLACK);
  if (king->isCheckmated()) {
    return Status::WHITE_WON;
  }
  return Status::NONE;
}

bool Engine::isStaleMate() const {
  const King* king = board_.getKing(Figure::WHITE);
  if (king->isStalemated()) {
    return true;
  }
  king = board_.getKing(Figure::BLACK);
  if (king->isStalemated()) {
    return true;
  }
  return false;
}

bool Engine::isDraw() const {
  std::vector<const Figure*> white_figures = board_.getFigures(Figure::WHITE);
  std::vector<const Figure*> black_figures = board_.getFigures(Figure::BLACK);
  assert(white_figures.size() > 0);
  if (white_figures.size() > 1 || black_figures.size() > 1) {
    return false;
  }
  assert(white_figures[0]->getType() == Figure::KING);
  assert(black_figures[0]->getType() == Figure::KING);
  return true;
}

int Engine::generateRandomValue(int max) const {
  return rand() % (max + 1);
}

Engine::Status Engine::makeMove(Figure::Color color) {
  Engine::Status status = getStatus();
  if (status != Status::NONE) {
    throw GameFinishedException(status);
  }
  std::vector<Figure::Move> all_moves;
  std::vector<const Figure*> figures = board_.getFigures(color);
  for (const Figure* figure: figures) {
    std::vector<Figure::Move> moves = figure->calculatePossibleMoves();
    all_moves.insert(all_moves.end(), moves.begin(), moves.end());
  }
  size_t moves_count = all_moves.size();
  debug_stream_ << "Moves count: " << moves_count << std::endl;

  auto move = all_moves[generateRandomValue(moves_count-1)];
  debug_stream_ << "My move (" << moves_count_ << "): " << move.old_field << "-" << move.new_field << std::endl;
  board_.makeMove(move);
  ++moves_count_;
  return getStatus();
}
