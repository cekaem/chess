#ifndef BOARD_H
#define BOARD_H

#include <array>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include "Field.h"
#include "Figure.h"

class Board {
 public:
  constexpr static size_t BoardSize = 8;

  struct NoFigureException : std::exception {
    NoFigureException(const Field& field) : field_(field) {}
    const Field field_;
  };

  struct FieldNotEmptyException : std::exception {
    FieldNotEmptyException(const Field& field, const Figure* figure)
      : field_(field), figure_(figure) {}
    const Field field_;
    const Figure* figure_;
  };

  struct IllegalMoveException : std::exception {
    IllegalMoveException(const Figure* figure, Field field)
      : figure_(figure), field_(field) {}
    const Figure* figure_;
    Field field_;
  };

  Board() noexcept;
  Board(const Board& other) noexcept;

  const Figure* addFigure(Figure::Type type, Field field, Figure::Color color)
      throw(FieldNotEmptyException);
  void removeFigure(Field field) throw(NoFigureException);
  const Figure* moveFigure(Field old_field, Field new_field, bool validate_move = true)
      throw(NoFigureException, IllegalMoveException);
  const Figure* getFigure(Field field) const noexcept;
  std::vector<const Figure*> getFigures() const noexcept;
  const auto& getFields() const noexcept { return fields_; }
  void setEnPassantPawn(Figure* pawn) noexcept { en_passant_pawn_ = pawn; }
  const Figure* getEnPassantPawn() const noexcept { return en_passant_pawn_; }
  const Figure* getKing(Figure::Color color) const noexcept;

  bool operator==(const Board& other) const noexcept;
  bool operator!=(const Board& other) const noexcept;

  friend std::ostream& operator<<(std::ostream& ostr, const Board& board);

 private:
  Board& operator=(const Board& other) = delete;
  Board(Board&& other) = delete;

  Figure* en_passant_pawn_{nullptr};
  bool validate_moves_{true};
  std::vector<std::unique_ptr<Figure>> figures_;
  std::array<std::array<Figure*, BoardSize>, BoardSize> fields_;
};

#endif  // BOARD_H
