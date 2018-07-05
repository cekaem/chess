#include "Board.h"

#include <algorithm>
#include <cassert>
#include <cstdlib>

int Board::number_of_copies_ = 0;

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

Board::Board() noexcept {
  std::array<Figure*, BoardSize> row;
  row.fill(nullptr);
  fields_.fill(row);
}

Board::Board(const Board& other) noexcept {
  ++number_of_copies_;
  std::array<Figure*, BoardSize> row;
  row.fill(nullptr);
  fields_.fill(row);

  const auto& figures = other.getFigures();
  for (const auto& figure : figures) {
    addFigure(figure->getType(), figure->getPosition(), figure->getColor());
  }
  in_analyze_mode_ = true;
}

bool Board::isMoveValid(Field old_field, Field new_field) {
  Figure* figure = fields_[old_field.letter][old_field.number];
  if (figure == nullptr) {
    return false;
  }

  try {
    GameStatus status = getGameStatus(figure->getColor());
    if (status != GameStatus::NONE) {
      return false;
    }
  } catch(const BadBoardStatusException&) {
    return false;
  }

  auto possible_moves = calculateMovesForFigure(figure);
  auto iter = std::find_if(possible_moves.begin(), possible_moves.end(),
      [new_field](const auto& m) -> bool {
        // TODO: check more than just new_field?
        return new_field == m.new_field;
      });

  return iter != possible_moves.end();
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

  assert(iter != figures_.end());
  std::unique_ptr<Figure> result = std::move(*iter);
  figures_.erase(iter);

  for (auto drawer : drawers_) {
    drawer->onFigureRemoved(field);
  }
  return result;
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

Board::ReversibleMoveWrapper Board::makeReversibleMove(Figure::Move move) {
  makeMove(move, true);
  return Board::ReversibleMoveWrapper(*this);
}

Board::GameStatus Board::makeMove(Figure::Move move, bool rev_mode) {
  Figure* figure = fields_[move.old_field.letter][move.old_field.number];
  if (figure == nullptr) {
    throw NoFigureException(move.old_field);
  }

  if (in_analyze_mode_ == false && isMoveValid(move.old_field, move.new_field) == false) {
    throw IllegalMoveException(figure, move.new_field);
  }

  std::unique_ptr<Figure> beaten_figure;
  if (fields_[move.new_field.letter][move.new_field.number] != nullptr) {
    beaten_figure = std::move(removeFigure(move.new_field));
    move.figure_beaten = true;
  }

  Figure::Color color = figure->getColor();

  // Handle en passant
  if (figure->getType() == Figure::PAWN && abs(move.new_field.number - move.old_field.number) == 2) {
    en_passant_pawn_ = static_cast<const Pawn*>(figure);
  } else {
    en_passant_pawn_ = nullptr;
  }

  // Handle castling
  if (move.castling != Figure::Move::Castling::NONE) {
    assert(figure->getType() == Figure::KING);
    Field::Letter old_l = move.castling == Figure::Move::Castling::KING_SIDE ? Field::H : Field::A;
    Field old_rook_position(old_l, move.old_field.number);
    Field::Letter new_l = move.castling == Figure::Move::Castling::KING_SIDE ? Field::F : Field::D;
    Field new_rook_position(new_l, move.old_field.number);
    moveFigure(old_rook_position, new_rook_position);
  }

  // Handle pawn promotion
  if (move.pawn_promotion != Figure::PAWN) {
    assert(figure->getType() == Figure::PAWN);
    const Pawn* pawn = static_cast<const Pawn*>(figure);
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
  if (in_analyze_mode_ == false) {
    move.is_check = isKingChecked(!color);
    move.is_mate = isKingCheckmated(!color);
  }
  for (auto drawer : drawers_) {
    drawer->onFigureMoved(move);
  }

  GameStatus status = GameStatus::NONE;
  if (in_analyze_mode_ == false) {
    status = getGameStatus(!color);
    if (status != GameStatus::NONE) {
      onGameFinished(status);
    }
  }

  if (rev_mode == true) {
    ReversibleMove reversible_move(move, std::move(beaten_figure));
    reversible_moves_.push_back(std::move(reversible_move));
  }

  return status;
}

bool Board::isKingChecked(Figure::Color color) {
  const King* king = getKing(color);
  if (king == nullptr) {
    return false;
  }
  const auto& figures = getFigures(!color);
  for (const auto& figure: figures) {
    auto moves = figure->calculatePossibleMoves();
    auto iter = std::find_if(moves.begin(), moves.end(),
        [king](const auto& move) -> bool {
          return move.new_field == king->getPosition();
        });
    if (iter != moves.end()) {
      return true;
    }
  }
  return false;
}

bool Board::isKingCheckmated(Figure::Color color) {
  const King* king = getKing(color);
  if (king == nullptr || isKingChecked(color) == false) {
    return false;
  }
  auto moves = king->calculatePossibleMoves();
  bool is_mate = true;
  for (const auto& move: moves) {
    auto wrapper = makeReversibleMove(move);
    is_mate = isKingChecked(color);
    if (is_mate == false) {
      break;
    }
  }
  return is_mate;
}

bool Board::isKingStalemated(Figure::Color color) {
  const King* king = getKing(color);
  if (king == nullptr || isKingChecked(color) == true) {
    return false;
  }
  auto moves = king->calculatePossibleMoves();
  bool is_stalemate = true;
  for (const auto& move: moves) {
    auto wrapper = makeReversibleMove(move);
    is_stalemate = isKingChecked(color);
    if (is_stalemate == false) {
      break;
    }
  }
  return is_stalemate;
}

Board::GameStatus Board::getGameStatus(Figure::Color color) {
  Board::GameStatus status = isCheckMate();
  if (status == GameStatus::WHITE_WON ||
      status == GameStatus::BLACK_WON) {
    return status;
  }
  if (isStaleMate(color) || isDraw()) {
    return GameStatus::DRAW;
  }
  if (isKingChecked(!color)) {
    throw BadBoardStatusException(this);
  }

  return GameStatus::NONE;
}

Board::GameStatus Board::isCheckMate() {
  const King* king = getKing(Figure::WHITE);
  if (king == nullptr) {
    throw BadBoardStatusException(this);
  }
  if (isKingCheckmated(Figure::WHITE)) {
    return GameStatus::BLACK_WON;
  }
  king = getKing(Figure::BLACK);
  if (king == nullptr) {
    throw BadBoardStatusException(this);
  }
  if (isKingCheckmated(Figure::BLACK)) {
    return GameStatus::WHITE_WON;
  }
  return GameStatus::NONE;
}

bool Board::isStaleMate(Figure::Color color) {
  const King* king = getKing(color);
  if (king == nullptr) {
    throw BadBoardStatusException(this);
  }
  return isKingStalemated(color);
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

Board::GameStatus Board::makeMove(Field old_field, Field new_field, Figure::Type promotion, bool rev_mode) {
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
  return makeMove(move, rev_mode);
}

bool Board::isMoveValid(Figure::Move& move, Figure::Color color) {
  if (move.castling == Figure::Move::Castling::KING_SIDE) {
    const Field::Number number = color == Figure::WHITE ? Field::ONE : Field::EIGHT;
    if (isKingChecked(color) == true) {
      return false;
    }
    Figure::Move m(move.old_field,
                   Field(static_cast<Field::Letter>(move.old_field.letter + 1), number),
                   Figure::Move::Castling::NONE);
    auto wrapper = makeReversibleMove(m);
    if (isKingChecked(color) == true) {
      return false;
    }
  } else if (move.castling == Figure::Move::Castling::QUEEN_SIDE) {
    const Field::Number number = color == Figure::WHITE ? Field::ONE : Field::EIGHT;
    if (isKingChecked(color) == true) {
      return false;
    }
    Figure::Move m(move.old_field,
                   Field(static_cast<Field::Letter>(move.old_field.letter - 1), number),
                   Figure::Move::Castling::NONE);
    auto wrapper = makeReversibleMove(m);
    if (isKingChecked(color) == true) {
      return false;
    }
  }

  auto wrapper = makeReversibleMove(move);
  if (isKingChecked(color) == true) {
    return false;
  }

  if (isKingChecked(!color)) {
    move.is_check = true;
  }
  if (isKingCheckmated(!color)) {
    move.is_mate = true;
  }
  return false;
}

std::vector<Figure::Move> Board::calculateMovesForFigure(const Figure* figure) {
  std::vector<Figure::Move> moves = figure->calculatePossibleMoves();
  moves.erase(std::remove_if(moves.begin(), moves.end(),
      [this, figure](auto& move) -> bool {
        return isMoveValid(move, figure->getColor()) == false;
      }));
  return moves;
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

void Board::undoLastReversibleMove() {
  assert(reversible_moves_.empty() == false);
  ReversibleMove reversible_move = std::move(reversible_moves_.back());
  reversible_moves_.pop_back();
  const Figure* figure = getFigure(reversible_move.new_field);
  if (reversible_move.promotion_move == true) {
    addFigure(Figure::PAWN, reversible_move.old_field, figure->getColor());
    removeFigure(reversible_move.new_field);
  } else {
    moveFigure(reversible_move.new_field, reversible_move.old_field);
  }
  Figure* bitten_figure = reversible_move.bitten_figure.get();
  if (bitten_figure != nullptr) {
    figures_.push_back(std::move(reversible_move.bitten_figure));
    Field old_position = bitten_figure->getPosition();
    fields_[old_position.letter][old_position.number] = bitten_figure;
  }
  if (reversible_move.castling_move == true) {
    Field::Number line = reversible_move.old_field.number;
    if (reversible_move.new_field.letter == Field::G) {
      moveFigure(Field(Field::F, line), Field(Field::H, line));
    } else {
      moveFigure(Field(Field::D, line), Field(Field::A, line));
    }
  }
}

void Board::undoAllReversibleMoves() {
  while (reversible_moves_.empty() == false) {
    undoLastReversibleMove();
  }
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
