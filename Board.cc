#include "Board.h"

#include <algorithm>

std::ostream& operator<<(std::ostream& ostr, const Board& board) {
  ostr << "{";
  for (const auto& row : board.fields_) {
    ostr << "{";
    for (const auto& item : row) {
      ostr << item << ", ";
    }
    ostr << "},";
  }
  ostr << "}";
  return ostr;
}

Board::Board() noexcept {
  // TODO: is it necessary?
  std::array<Figure*, BoardSize> row;
  row.fill(nullptr);
  fields_.fill(row);
}

Board::Board(const Board& other) noexcept {
  const auto& figures = other.getFigures();
  for (const auto& figure : figures) {
    addFigure(figure->getType(), figure->getPosition(), figure->getColor());
  }
}

bool Board::operator==(const Board& other) const noexcept {
  for (size_t i = 0; i < BoardSize; ++i) {
    for (size_t j = 0; j < BoardSize; ++j) {
      const Figure* f1 = fields_[i][j];
      const Figure* f2 = other.fields_[i][j];
      if (f1 == nullptr) {
        if (f2 == nullptr) {
          continue;
        } else {
          return false;
        }
      } else {
        if (f2 == nullptr) {
          return false;
        }
      }
      if (*fields_[i][j] != *fields_[i][j]) {
        return false;
      }
    }
  }
  return true;
}

bool Board::operator!=(const Board& other) const noexcept {
  return !(*this == other);
}

void Board::addFigure(Figure::Type type, Field field, Figure::Color color)
    throw(FieldNotEmptyException) {
  const Figure* old_figure = fields_[field.letter][field.number];
  if (old_figure != nullptr) {
    throw FieldNotEmptyException(field, old_figure);
  }
  auto figure = FiguresFactory::GetFiguresFactory().createFigure(type, *this, field, color);
  fields_[field.letter][field.number] = figure.get();
  figures_.push_back(std::move(figure));
}

void Board::removeFigure(Field field) throw(NoFigureException) {
  const Figure* figure = fields_[field.letter][field.number];
  if (figure == nullptr) {
    throw NoFigureException(field);
  }
  fields_[field.letter][field.number] = nullptr;
  figures_.erase(std::find_if(figures_.begin(), figures_.end(),
        [figure](const auto& iter) -> bool {
          return iter.get() == figure;
        }));
}

const Figure* Board::moveFigure(Field old_field, Field new_field)
    throw(Board::NoFigureException, Figure::IllegalMoveException) {
  Figure* figure = fields_[old_field.letter][old_field.number];
  if (figure == nullptr) {
    throw NoFigureException(old_field);
  }
  const Figure* bitten_figure = fields_[new_field.letter][new_field.number];
  fields_[new_field.letter][new_field.number] = figure;
  figure->move(new_field);
  return bitten_figure;
}

const Figure* Board::getFigure(Field field) const noexcept {
  return fields_[field.letter][field.number];
}
