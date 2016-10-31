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

class Pawn;

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

  Board() noexcept;
  Board(const Board& other) noexcept;

  void addFigure(Figure::Type type, Field field, Figure::Color color)
      throw(FieldNotEmptyException);
  void removeFigure(Field field) throw(NoFigureException);
  const Figure* moveFigure(Field old_field, Field new_field)
      throw(NoFigureException, Figure::IllegalMoveException);
  const Figure* getFigure(Field field) const noexcept;
  const std::vector<std::unique_ptr<Figure>>& getFigures() const { return figures_; }
  const auto& getFields() const { return fields_; }
  void setEnPassantPawn(Pawn* pawn) { enPassantPawn_ = pawn; }
  const Pawn* getEnPassantPawn() const { return enPassantPawn_; }

  bool operator==(const Board& other) const noexcept;
  bool operator!=(const Board& other) const noexcept;

  friend std::ostream& operator<<(std::ostream& ostr, const Board& board);

 private:
  Board& operator=(const Board& other) = delete;
  Board(Board&& other) = delete;

  Pawn* enPassantPawn_{nullptr};
  std::vector<std::unique_ptr<Figure>> figures_;
  std::array<std::array<Figure*, BoardSize>, BoardSize> fields_;
};

#endif  // BOARD_H
