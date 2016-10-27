#include "Board.h"

Board::Board() noexcept {
  // TODO: is it necessary?
  std::array<const Figure*, BoardSize> row;
  row.fill(nullptr);
  fields_.fill(row);
}

void Board::validateField(Field field) const throw (WrongFieldException) {
  if (field.first <= 0 || field.first > BoardSize ||
      field.second <= 0 || field.second > BoardSize) {
    throw WrongFieldException(field);
  }
}

void Board::addFigure(const Figure* figure, Field field)
    throw(WrongFieldException, NoFigureException, FieldNotEmptyException) {
  validateField(field);
  if (figure == nullptr) {
    throw NoFigureException(field);
  }
  const Figure* old_figure = fields_[field.first - 1][field.second - 1];
  if (old_figure != nullptr) {
    throw FieldNotEmptyException(field, old_figure);
  }
  fields_[field.first - 1][field.second - 1] = figure;
}

const Figure* Board::removeFigure(Field field)
    throw(WrongFieldException, NoFigureException) {
  validateField(field);
  const Figure* figure = fields_[field.first - 1][field.second - 1];
  if (figure == nullptr) {
    throw NoFigureException(field);
  }
  fields_[field.first - 1][field.second - 1] = nullptr;
  return figure;
}

const Figure* Board::getFigure(Field field) const throw(WrongFieldException) {
  validateField(field);
  return fields_[field.first - 1][field.second - 1];
}
