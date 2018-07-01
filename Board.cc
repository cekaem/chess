#include "Board.h"

#include <algorithm>
#include <cassert>

std::ostream& operator<<(std::ostream& ostr, Board::GameStatus status) {
  ostr << static_cast<int>(status);
  return ostr;
}

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

Board::Board(bool validate_moves) noexcept : validate_moves_(validate_moves) {
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

void Board::removeFigure(Field field) {
  const Figure* figure = fields_[field.letter][field.number];
  if (figure == nullptr) {
    throw NoFigureException(field);
  }
  fields_[field.letter][field.number] = nullptr;
  auto iter = std::find_if(figures_.begin(), figures_.end(),
        [figure](const auto& iter) -> bool {
          return iter.get() == figure;
        });

  figures_.erase(iter);

  for (auto drawer : drawers_) {
    drawer->onFigureRemoved(field);
  }
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
    GameStatus status = getGameStatus(figure->getColor());
    if (status != GameStatus::NONE) {
      throw IllegalMoveException(figure, move.new_field);
    }
  }

  Figure::Color color = figure->getColor();
  if (validate_moves_) {
    auto possible_moves = figure->calculatePossibleMoves();
    auto iter = std::find_if(possible_moves.begin(), possible_moves.end(),
        [move](const auto& m) -> bool {
          // TODO: check more than just new_field
          return move.new_field == m.new_field;
        });
    if (iter == possible_moves.end()) {
      throw IllegalMoveException(figure, move.new_field);
    }
  }

  const Figure* beaten_figure = fields_[move.new_field.letter][move.new_field.number];
  if (beaten_figure) {
    removeFigure(move.new_field);
    move.figure_beaten = true;
  }

  // Handle pawn promotion
  if (move.pawn_promotion != Figure::PAWN) {
    const Figure* f = getFigure(move.old_field);
    assert(f->getType() == Figure::PAWN);
    const Pawn* pawn = static_cast<const Pawn*>(f);
    addFigure(move.pawn_promotion, move.new_field, pawn->getColor());
    removeFigure(move.old_field);
    figure = nullptr;
  }

  if (figure != nullptr) {
    figure->move(move);
    fields_[move.new_field.letter][move.new_field.number] = figure;
  }
  fields_[move.old_field.letter][move.old_field.number] = nullptr;

  // Update fields is_check and is_mate
  const King* king = static_cast<const King*>(getKing(!color));
  if (king && validate_moves_) {
    move.is_check = king->isChecked();
    move.is_mate = king->isCheckmated();
  }
  for (auto drawer : drawers_) {
    drawer->onFigureMoved(move);
  }

  if (validate_moves_) {
    GameStatus status = getGameStatus(!color);
    if (status != GameStatus::NONE) {
      onGameFinished(status);
    }
  }
}

Board::GameStatus Board::getGameStatus(Figure::Color color) const {
  Board::GameStatus status = isCheckMate();
  if (status == GameStatus::WHITE_WON ||
      status == GameStatus::BLACK_WON) {
    return status;
  }
  if (isStaleMate(color) || isDraw()) {
    return GameStatus::DRAW;
  }
  return GameStatus::NONE;
}

Board::GameStatus Board::isCheckMate() const {
  const King* king = getKing(Figure::WHITE);
  if (king == nullptr) {
    throw BadBoardStatusException(this);
  }
  if (king->isCheckmated()) {
    return GameStatus::BLACK_WON;
  }
  king = getKing(Figure::BLACK);
  if (king == nullptr) {
    throw BadBoardStatusException(this);
  }
  if (king->isCheckmated()) {
    return GameStatus::WHITE_WON;
  }
  return GameStatus::NONE;
}

bool Board::isStaleMate(Figure::Color color) const {
  const King* king = getKing(color);
  if (king == nullptr) {
    throw BadBoardStatusException(this);
  }
  return king->isStalemated();
}

bool Board::isDraw() const {
  std::vector<const Figure*> white_figures = getFigures(Figure::WHITE);
  std::vector<const Figure*> black_figures = getFigures(Figure::BLACK);
  assert(white_figures.size() > 0);
  if (white_figures.size() > 1 || black_figures.size() > 1) {
    return false;
  }
  if (white_figures[0]->getType() != Figure::KING ||
      black_figures[0]->getType() != Figure::KING) {
    throw BadBoardStatusException(this);
  }
  return true;
}

void Board::makeMove(Field old_field, Field new_field, Figure::Type promotion) {
  Figure* figure = fields_[old_field.letter][old_field.number];
  if (figure == nullptr) {
    throw NoFigureException(old_field);
  }

  Figure::Move::Castling castling = Figure::Move::isCastling(this, old_field, new_field);
  if (castling != Figure::Move::Castling::NONE) {
    if (figure->getType() != Figure::KING) {
      throw IllegalMoveException(figure, new_field);
    }
    const King* king = static_cast<const King*>(figure);
    if (king->canCastle(castling) == false) {
      throw IllegalMoveException(figure, new_field);
    }
  }
  bool is_promotion = Figure::Move::isPromotion(this, old_field, new_field);
  if ((is_promotion == true && promotion == Figure::PAWN) ||
      (is_promotion == false && promotion != Figure::PAWN)) {
    throw IllegalMoveException(figure, new_field);
  }
  Figure::Move move(old_field, new_field, false, false, castling, false, promotion);
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

void Board::onGameFinished(Board::GameStatus status) noexcept {
  for (auto* drawer: drawers_) {
    drawer->onGameFinished(status);
  }
}

void Board::addBoardDrawer(BoardDrawer* drawer) noexcept {
  drawers_.push_back(drawer);
}

void Board::removeBoardDrawer(BoardDrawer* drawer) noexcept {
  drawers_.erase(std::remove(drawers_.begin(), drawers_.end(), drawer), drawers_.end());
}
