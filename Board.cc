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
  std::array<Figure*, BoardSize> row;
  row.fill(nullptr);
  fields_.fill(row);
}

Board::Board(const Board& other) noexcept {
  std::array<Figure*, BoardSize> row;
  row.fill(nullptr);
  fields_.fill(row);

  const auto& figures = other.getFigures();
  for (const auto& figure : figures) {
    addFigure(figure->getType(), figure->getPosition(), figure->getColor());
  }
  validate_moves_ = false;
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
      if (*fields_[i][j] != *other.fields_[i][j]) {
        return false;
      }
    }
  }
  return true;
}

bool Board::operator!=(const Board& other) const noexcept {
  return !(*this == other);
}

const Figure* Board::addFigure(Figure::Type type, Field field, Figure::Color color) {
  const Figure* old_figure = fields_[field.letter][field.number];
  if (old_figure != nullptr) {
    throw FieldNotEmptyException(field, old_figure);
  }
  auto figure = FiguresFactory::GetFiguresFactory().createFigure(type, *this, field, color);
  Figure* new_figure = figure.get();
  fields_[field.letter][field.number] = new_figure;
  figures_.push_back(std::move(figure));
  for (auto drawer : drawers_) {
    drawer->onFigureAdded(type, color, field);
  }
  return new_figure;  
}

std::unique_ptr<Figure> Board::removeFigure(Field field) {
  const Figure* figure = fields_[field.letter][field.number];
  if (figure == nullptr) {
    throw NoFigureException(field);
  }
  fields_[field.letter][field.number] = nullptr;
  auto iter = std::find_if(figures_.begin(), figures_.end(),
        [figure](const auto& iter) -> bool {
          return iter.get() == figure;
        });
  std::unique_ptr<Figure> result = std::move(*iter);
  figures_.erase(iter);

  for (auto drawer : drawers_) {
    drawer->onFigureRemoved(field);
  }

  return std::move(result);
}

std::unique_ptr<Figure> Board::moveFigure(Field old_field, Field new_field, bool validate_move) {
  Figure* figure = fields_[old_field.letter][old_field.number];
  if (figure == nullptr) {
    throw NoFigureException(old_field);
  }
  if (validate_moves_ && validate_move) {
    auto possible_moves = figure->calculatePossibleMoves();
    auto iter = std::find_if(possible_moves.begin(), possible_moves.end(),
        [new_field](const auto& iter) -> bool {
          return new_field == iter.first;
        });
    if (iter == possible_moves.end()) {
      throw IllegalMoveException(figure, new_field);
    }
  }

  std::unique_ptr<Figure> result;
  const Figure* bitten_figure = fields_[new_field.letter][new_field.number];
  if (bitten_figure) {
    result = removeFigure(new_field);
  }
  figure->move(new_field);
  fields_[new_field.letter][new_field.number] = figure;
  fields_[old_field.letter][old_field.number] = nullptr;
  for (auto drawer : drawers_) {
    drawer->onFigureMoved(old_field, new_field);
  }
  return std::move(result);
}

const Figure* Board::getFigure(Field field) const noexcept {
  return fields_[field.letter][field.number];
}

const Figure* Board::getKing(Figure::Color color) const noexcept {
  for (size_t i = 0; i < BoardSize; ++i) {
    for (size_t j = 0; j < BoardSize; ++j) {
      const Figure* figure = fields_[i][j];
      if (figure && figure->getType() == Figure::KING && figure->getColor() == color) {
        return figure;
      }
    }
  }
  return nullptr;
}

void Board::addBoardDrawer(BoardDrawer* drawer) noexcept {
  drawers_.push_back(drawer);
}

void Board::removeBoardDrawer(BoardDrawer* drawer) noexcept {
  drawers_.erase(std::remove(drawers_.begin(), drawers_.end(), drawer), drawers_.end());
}
