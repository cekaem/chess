#ifndef FIGURE_H
#define FIGURE_H

#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include "Field.h"

class Board;

static const int PAWN_VALUE = 1;
static const int KNIGHT_VALUE = 3;
static const int BISHOP_VALUE = 3;
static const int ROOK_VALUE = 5;
static const int QUEEN_VALUE = 8;
static const int KING_VALUE = 0;

class Figure {
 public:
  enum Type {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING};
  enum Color {WHITE, BLACK};

  struct IllegalMoveException : std::exception {
    IllegalMoveException(const Figure* figure, Field field)
      : figure_(figure), field_(field) {}
    const Figure* figure_;
    Field field_;
  };

  using Move = std::pair<Field, const Figure*>;

  Color getColor() const { return color_; }
  Field& getPosition() const { return field_; }
  virtual int getValue() const { return value_; }
  bool movedAtLeastOnce() const { return moved_at_least_once_; }

  virtual std::vector<Move> calculatePossibleMoves() const = 0;
  virtual Type getType() const = 0;
  void validateMoves(bool validate) const { validate_moves_ = validate; }
  virtual void move(Field field) throw(IllegalMoveException);

  bool operator==(const Figure& other) const;
  bool operator!=(const Figure& other) const;

 protected:
  Figure(Board& board, Field field, Color color, int value) noexcept;

  Board& board_;
  mutable Field field_;

 private:
  const Color color_;
  const int value_;
  mutable bool validate_moves_{true};
  bool moved_at_least_once_{false};
};


class Pawn : public Figure {
 public:
  Pawn(Board& board, Field field, Color color) noexcept
    : Figure(board, field, color, PAWN_VALUE) {}
  void move(Field field) throw(IllegalMoveException) override;
  std::vector<Move> calculatePossibleMoves() const override;
  Type getType() const override { return PAWN; }
};

class Knight : public Figure {
 public:
  Knight(Board& board, Field field, Color color) noexcept
    : Figure(board, field, color, KNIGHT_VALUE) {}
  std::vector<Move> calculatePossibleMoves() const override;
  Type getType() const override { return KNIGHT; }
};

class Bishop : public Figure {
 public:
  Bishop(Board& board, Field field, Color color) noexcept
    : Figure(board, field, color, BISHOP_VALUE) {}
  std::vector<Move> calculatePossibleMoves() const override;
  Type getType() const override { return BISHOP; }
};

class Rook : public Figure {
 public:
  Rook(Board& board, Field field, Color color) noexcept
    : Figure(board, field, color, ROOK_VALUE) {}
  std::vector<Move> calculatePossibleMoves() const override;
  Type getType() const override { return ROOK; }
};

class Queen : public Figure {
 public:
  Queen(Board& board, Field field, Color color) noexcept
    : Figure(board, field, color, QUEEN_VALUE) {}
  std::vector<Move> calculatePossibleMoves() const override;
  Type getType() const override { return QUEEN; }
};

class King : public Figure {
 public:
  King(Board& board, Field field, Color color) noexcept
    : Figure(board, field, color, KING_VALUE) {}
  std::vector<Move> calculatePossibleMoves() const override;
  void move(Field field) throw(IllegalMoveException) override;
  Type getType() const override { return KING; }
  bool isChecked() const;

 private:
  void addPossibleCastlings(std::vector<Move>& moves) const;
  mutable bool look_for_enemies_moves_{true};
};

class FiguresFactory {
 public:
  static FiguresFactory& GetFiguresFactory() noexcept;
  std::unique_ptr<Figure> createFigure
      (Figure::Type type, Board& board, Field field, Figure::Color color) noexcept;

 private:
  friend class Figure;

  FiguresFactory() noexcept;
};

#endif  // FIGURE_H
