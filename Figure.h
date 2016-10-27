#ifndef FIGURE_H
#define FIGURE_H

#include <stdexcept>
#include <vector>

#include "Board.h"

static const int PAWN_VALUE = 1;
static const int KNIGHT_VALUE = 3;
static const int BISHOP_VALUE = 3;
static const int ROOK_VALUE = 5;
static const int QUEEN_VALUE = 8;
static const int KING_VALUE = 0;

class Figure {
 public:
  enum Color {WHITE, BLACK};

  struct IllegalMoveException : std::exception {
    IllegalMoveException(const Figure* figure, Board::Field field)
      : figure_(figure), field_(field) {}
    const Figure* figure;
    Board::Field field;
  };

  Figure(const Board& board, Board::Field field, Color color, int value) noexcept
    : board_(board), color_(color), value_(value) {}

  int getValue() const { return value_; }

  void move(Board::Field field) throw(IllegalMoveException);

  virtual std::vector<Board::Field> getPossibleMoves() const = 0;
 
 protected:
  const Board& board_;
  Board::Field field_;
  const Color color_;
  const int value_;
};

class Pawn : public Figure {
 public:
  Pawn(const Board& board, Board::Field field, Color color)
    : Figure(board, field, color, PAWN_VALUE) {}
  std::vector<Board::Field> getPossibleMoves() const override;
};

class Knight : public Figure {
 public:
  Knight(const Board& board, Board::Field field, Color color)
    : Figure(board, field, color, KNIGHT_VALUE) {}
  std::vector<Board::Field> getPossibleMoves() const override;
};

class Bishop : public Figure {
 public:
  Bishop(const Board& board, Board::Field field, Color color)
    : Figure(board, field, color, BISHOP_VALUE) {}
  std::vector<Board::Field> getPossibleMoves() const override;
};

class Rook : public Figure {
 public:
  Rook(const Board& board, Board::Field field, Color color)
    : Figure(board, field, color, ROOK_VALUE) {}
  std::vector<Board::Field> getPossibleMoves() const override;
};

class Queen : public Figure {
 public:
  Queen(const Board& board, Board::Field field, Color color)
    : Figure(board, field, color, QUEEN_VALUE) {}
  std::vector<Board::Field> getPossibleMoves() const override;
};

class King : public Figure {
 public:
  King(const Board& board, Board::Field field, Color color)
    : Figure(board, field, color, KING_VALUE) {}
  std::vector<Board::Field> getPossibleMoves() const override;
};

#endif  // FIGURE_H
