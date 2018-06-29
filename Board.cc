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

void Board::moveFigure(Field old_field, Field new_field) {
  Figure* figure = fields_[old_field.letter][old_field.number];
  if (figure == nullptr) {
    throw NoFigureException(old_field);
  }
  if (fields_[new_field.letter][new_field.number] != nullptr) {
    throw FieldNotEmptyException(new_field, figure);
  }
  fields_[new_field.letter][new_field.number] = figure;
  fields_[old_field.letter][old_field.number] = nullptr;
  figure->move(new_field);
}

void Board::makeMove(Figure::Move move) {
  Figure* figure = fields_[move.old_field.letter][move.old_field.number];
  if (figure == nullptr) {
    throw NoFigureException(move.old_field);
  }
  if (validate_moves_) {
    auto possible_moves = figure->calculatePossibleMoves();
    auto iter = std::find_if(possible_moves.begin(), possible_moves.end(),
        [move](const auto& m) -> bool {
          return move.new_field == m.new_field;
        });
    if (iter == possible_moves.end()) {
      throw IllegalMoveException(figure, move.new_field);
    }
  }

  const Figure* beaten_figure = fields_[move.new_field.letter][move.new_field.number];
  if (beaten_figure) {
    removeFigure(move.new_field);
  }
  figure->move(move);
  fields_[move.new_field.letter][move.new_field.number] = figure;
  fields_[move.old_field.letter][move.old_field.number] = nullptr;

  for (auto drawer : drawers_) {
    drawer->onFigureMoved(move);
  }
}

void Board::makeMove(Field old_field, Field new_field, Figure::Type promotion) {
  // TODO: add castling handling
  Figure::Move move(old_field, new_field, false, false, Figure::Move::Castling::NONE, nullptr, promotion);
  Figure* figure = fields_[old_field.letter][old_field.number];
  if (figure == nullptr) {
    throw NoFigureException(old_field);
  }
  figure->lookForKingUnveils(false);
  figure->updateMove(move);  // for updating fields is_check and is_mate
  figure->lookForKingUnveils(true);
  makeMove(move);
}

const Figure* Board::getFigure(Field field) const noexcept {
  return fields_[field.letter][field.number];
}

std::vector<const Figure*> Board::getFigures(Figure::Color color) const noexcept {
  std::vector<const Figure*> figures;
  for (const auto& figure: figures_) {
    if (figure->getColor() == color) {
      figures.push_back(figure.get());
    }
  }
  return figures;
}

const King* Board::getKing(Figure::Color color) const noexcept {
  for (size_t i = 0; i < BoardSize; ++i) {
    for (size_t j = 0; j < BoardSize; ++j) {
      const Figure* figure = fields_[i][j];
      if (figure && figure->getType() == Figure::KING && figure->getColor() == color) {
        return static_cast<const King*>(figure);
      }
    }
  }
  return nullptr;
}

void Board::setStandardBoard() {
  addFigure(Figure::PAWN, Field(Field::A, Field::TWO), Figure::WHITE);
  addFigure(Figure::PAWN, Field(Field::B, Field::TWO), Figure::WHITE);
  addFigure(Figure::PAWN, Field(Field::C, Field::TWO), Figure::WHITE);
  addFigure(Figure::PAWN, Field(Field::D, Field::TWO), Figure::WHITE);
  addFigure(Figure::PAWN, Field(Field::E, Field::TWO), Figure::WHITE);
  addFigure(Figure::PAWN, Field(Field::F, Field::TWO), Figure::WHITE);
  addFigure(Figure::PAWN, Field(Field::G, Field::TWO), Figure::WHITE);
  addFigure(Figure::PAWN, Field(Field::H, Field::TWO), Figure::WHITE);
  addFigure(Figure::ROOK, Field(Field::A, Field::ONE), Figure::WHITE);
  addFigure(Figure::ROOK, Field(Field::H, Field::ONE), Figure::WHITE);
  addFigure(Figure::KNIGHT, Field(Field::B, Field::ONE), Figure::WHITE);
  addFigure(Figure::KNIGHT, Field(Field::G, Field::ONE), Figure::WHITE);
  addFigure(Figure::BISHOP, Field(Field::C, Field::ONE), Figure::WHITE);
  addFigure(Figure::BISHOP, Field(Field::F, Field::ONE), Figure::WHITE);
  addFigure(Figure::QUEEN, Field(Field::D, Field::ONE), Figure::WHITE);
  addFigure(Figure::KING, Field(Field::E, Field::ONE), Figure::WHITE);

  addFigure(Figure::PAWN, Field(Field::A, Field::SEVEN), Figure::BLACK);
  addFigure(Figure::PAWN, Field(Field::B, Field::SEVEN), Figure::BLACK);
  addFigure(Figure::PAWN, Field(Field::C, Field::SEVEN), Figure::BLACK);
  addFigure(Figure::PAWN, Field(Field::D, Field::SEVEN), Figure::BLACK);
  addFigure(Figure::PAWN, Field(Field::E, Field::SEVEN), Figure::BLACK);
  addFigure(Figure::PAWN, Field(Field::F, Field::SEVEN), Figure::BLACK);
  addFigure(Figure::PAWN, Field(Field::G, Field::SEVEN), Figure::BLACK);
  addFigure(Figure::PAWN, Field(Field::H, Field::SEVEN), Figure::BLACK);
  addFigure(Figure::ROOK, Field(Field::A, Field::EIGHT), Figure::BLACK);
  addFigure(Figure::ROOK, Field(Field::H, Field::EIGHT), Figure::BLACK);
  addFigure(Figure::KNIGHT, Field(Field::B, Field::EIGHT), Figure::BLACK);
  addFigure(Figure::KNIGHT, Field(Field::G, Field::EIGHT), Figure::BLACK);
  addFigure(Figure::BISHOP, Field(Field::C, Field::EIGHT), Figure::BLACK);
  addFigure(Figure::BISHOP, Field(Field::F, Field::EIGHT), Figure::BLACK);
  addFigure(Figure::QUEEN, Field(Field::D, Field::EIGHT), Figure::BLACK);
  addFigure(Figure::KING, Field(Field::E, Field::EIGHT), Figure::BLACK);
}

void Board::addBoardDrawer(BoardDrawer* drawer) noexcept {
  drawers_.push_back(drawer);
}

void Board::removeBoardDrawer(BoardDrawer* drawer) noexcept {
  drawers_.erase(std::remove(drawers_.begin(), drawers_.end(), drawer), drawers_.end());
}
