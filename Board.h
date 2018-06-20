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

class BoardDrawer {
 public:
  virtual void onFigureAdded(Figure::Type type, Figure::Color color, Field field) = 0;
  virtual void onFigureRemoved(Field field) = 0;
  virtual void onFigureMoved(Field start_field, Field end_field) = 0;
};

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

  const Figure* addFigure(Figure::Type type, Field field, Figure::Color color);
  std::unique_ptr<Figure> removeFigure(Field field);
  std::unique_ptr<Figure> moveFigure(Field old_field, Field new_field, bool validate_move = true);
  const Figure* getFigure(Field field) const noexcept;
  const std::vector<std::unique_ptr<Figure>>& getFigures() const noexcept { return figures_; }
  const auto& getFields() const noexcept { return fields_; }
  void setEnPassantPawn(Figure* pawn) noexcept { en_passant_pawn_ = pawn; }
  const Figure* getEnPassantPawn() const noexcept { return en_passant_pawn_; }
  const Figure* getKing(Figure::Color color) const noexcept;
  void addBoardDrawer(BoardDrawer* drawer) noexcept;
  void removeBoardDrawer(BoardDrawer* drawer) noexcept;

  bool operator==(const Board& other) const noexcept;
  bool operator!=(const Board& other) const noexcept;

  friend std::ostream& operator<<(std::ostream& ostr, const Board& board);

 private:
  Board& operator=(const Board& other) = delete;
  Board(Board&& other) = delete;

  Figure* en_passant_pawn_{nullptr};
  bool validate_moves_{true};
  std::vector<std::unique_ptr<Figure>> figures_;
  std::vector<BoardDrawer*> drawers_;
  std::array<std::array<Figure*, BoardSize>, BoardSize> fields_;
};

#endif  // BOARD_H
