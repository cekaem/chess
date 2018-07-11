#ifndef FIGURE_H
#define FIGURE_H

#include <memory>
#include <ostream>
#include <stdexcept>
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

  struct Move {
    enum class Castling {NONE, QUEEN_SIDE, KING_SIDE};

    Move(Field ofield, Field nfield, bool check, bool mate,
         Castling cast, bool beaten, Type promo) :
      old_field(ofield), new_field(nfield), is_check(check), is_mate(mate),
      castling(cast), figure_beaten(beaten), pawn_promotion(promo) {}

    static Castling isCastling(const Board* board, Field old_field, Field new_field);
    static bool isPromotion(const Board* board, Field old_field, Field new_field);
    static bool isEnPassant(const Board* board, Field old_field, Field new_field);

    Move() {}

    Move(Field field) : new_field(field) {}

    Move(Field ofield, Field nfield, Castling cast)
        : old_field(ofield), new_field(nfield), castling(cast) {}

    bool operator==(const Move& other) const;
    friend std::ostream& operator<<(std::ostream& ostr, const Move& move);
    
    Field old_field;
    Field new_field;
    bool is_check{false};
    bool is_mate{false};
    Castling castling{Castling::NONE};
    bool figure_beaten{false};
    Type pawn_promotion{PAWN};
  };

  Color getColor() const { return color_; }
  Field getPosition() const { return field_; }
  void setPosition(const Field& field) { field_ = field; }
  virtual int getValue() const { return value_; }
  bool movedAtLeastOnce() const { return moved_at_least_once_; }
  void setMovedAtLeastOnce(bool moved) { moved_at_least_once_ = moved; }

  virtual std::vector<Move> calculatePossibleMoves() const = 0;
  virtual Type getType() const = 0;

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
};

std::ostream& operator<<(std::ostream& ostr, const Figure::Move& move);
std::ostream& operator<<(std::ostream& ostr, const std::vector<Figure::Move>& moves);

inline Figure::Color operator!(Figure::Color color) {
  return (color == Figure::WHITE) ? Figure::BLACK : Figure::WHITE;
}

class Pawn : public Figure {
 public:
  Pawn(Board& board, Field field, Color color) noexcept
    : Figure(board, field, color, PAWN_VALUE) {}
  std::vector<Move> calculatePossibleMoves() const override;
  Type getType() const override { return PAWN; }

 private:
  bool canPromote() const;
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
  Type getType() const override { return KING; }
  bool canCastle(Figure::Move::Castling castling) const;

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
