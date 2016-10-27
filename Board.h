#ifndef BOARD_H
#define BOARD_H

#include <array>
#include <stdexcept>
#include <utility>

class Figure;

class Board {
 public:
  constexpr static size_t BoardSize = 8;
  enum Letter {A = 1, B, C, D, E, F, G, H};
  using Field = std::pair<Letter, unsigned>;

  struct WrongFieldException : std::exception {
     WrongFieldException(const Field& field) : field_(field) {}
     const Field field_;
  };

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

  void addFigure(const Figure* figure, Field field)
      throw(WrongFieldException, NoFigureException, FieldNotEmptyException);
  const Figure* removeFigure(Field field) throw(WrongFieldException, NoFigureException);
  const Figure* getFigure(Field field) const throw(WrongFieldException);

 private:
  void validateField(Field field) const throw (WrongFieldException);

  std::array<std::array<const Figure*, BoardSize>, BoardSize> fields_;
};

#endif  // BOARD_H
