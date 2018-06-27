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

  using Move = std::pair<Field, Field>;

  Color getColor() const { return color_; }
  Field getPosition() const { return field_; }
  virtual int getValue() const { return value_; }
  bool movedAtLeastOnce() const { return moved_at_least_once_; }
  void lookForKingUnveils(bool look) const { look_for_king_unveils_ = look; }
  bool looksForKingUnveils() const { return look_for_king_unveils_; }

  virtual std::vector<Move> calculatePossibleMoves() const = 0;
  virtual Type getType() const = 0;
  virtual void move(Field field);

  bool operator==(const Figure& other) const;
  bool operator!=(const Figure& other) const;

 protected:
  Figure(Board& board, Field field, Color color, int value) noexcept;

  Board& board_;
  Field field_;

 private:
  const Color color_;
  const int value_;
  bool moved_at_least_once_{false};
  mutable bool look_for_king_unveils_{true};
};

inline Figure::Color operator!(Figure::Color color) {
  return (color == Figure::WHITE) ? Figure::BLACK : Figure::WHITE;
}

class Pawn : public Figure {
 public:
  Pawn(Board& board, Field field, Color color) noexcept
    : Figure(board, field, color, PAWN_VALUE) {}
  void move(Field field) override;
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
  void move(Field field) override;
  Type getType() const override { return KING; }
  bool isChecked() const;
  bool isCheckmated() const;
  bool isStalemated() const;

 private:
  void addPossibleCastlings(std::vector<Move>& moves) const;
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
