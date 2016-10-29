#ifndef FIGURE_H
#define FIGURE_H

#include <stdexcept>
#include <utility>
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

  using Move = std::pair<Board::Field, const Figure*>;

  struct IllegalMoveException : std::exception {
    IllegalMoveException(const Figure* figure, Board::Field field)
      : figure_(figure), field_(field) {}
    const Figure* figure_;
    Board::Field field_;
  };

  Figure(Board& board, Board::Field field, Color color, int value)
    throw(Board::WrongFieldException, Board::FieldNotEmptyException);

  int getValue() const { return value_; }

  Color getColor() const { return color_; }

  virtual void move(Board::Field field) throw(IllegalMoveException);

  virtual std::vector<Move> calculatePossibleMoves() const = 0;
 
 protected:
  Board& board_;
  Board::Field field_;
  const Color color_;
  const int value_;
};

class Pawn : public Figure {
 public:
  Pawn(Board& board, Board::Field field, Color color) noexcept
    : Figure(board, field, color, PAWN_VALUE) {}
  void move(Board::Field field) throw(IllegalMoveException) override;
  std::vector<Move> calculatePossibleMoves() const override;
};

class Knight : public Figure {
 public:
  Knight(Board& board, Board::Field field, Color color) noexcept
    : Figure(board, field, color, KNIGHT_VALUE) {}
  std::vector<Move> calculatePossibleMoves() const override;
};

class Bishop : public Figure {
 public:
  Bishop(Board& board, Board::Field field, Color color) noexcept
    : Figure(board, field, color, BISHOP_VALUE) {}
  std::vector<Move> calculatePossibleMoves() const override;
};

class Rook : public Figure {
 public:
  Rook(Board& board, Board::Field field, Color color) noexcept
    : Figure(board, field, color, ROOK_VALUE) {}
  std::vector<Move> calculatePossibleMoves() const override;
};

class Queen : public Figure {
 public:
  Queen(Board& board, Board::Field field, Color color) noexcept
    : Figure(board, field, color, QUEEN_VALUE) {}
  std::vector<Move> calculatePossibleMoves() const override;
};

class King : public Figure {
 public:
  King(Board& board, Board::Field field, Color color) noexcept
    : Figure(board, field, color, KING_VALUE) {}
  std::vector<Move> calculatePossibleMoves() const override;
};

#endif  // FIGURE_H
