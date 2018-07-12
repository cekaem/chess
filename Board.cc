#include "Board.h"

#include <algorithm>
#include <cassert>

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

  if (other.en_passant_pawn_ != nullptr) {
    en_passant_pawn_ = static_cast<const Pawn*>(getFigure(other.en_passant_pawn_->getPosition()));
  }
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
  figure->setPosition(new_field);
}

void Board::updateCastlings(const Figure::Move& move) {
  Field::Letter letter = move.old_field.letter;
  Field::Number number = move.old_field.number;
  if (number != Field::ONE && number != Field::EIGHT) {
    return;
  }
  switch (letter) {
    case Field::A: {
      if (number == Field::ONE) {
        castlings_[static_cast<size_t>(Figure::Move::Castling::Q)] = false;
      } else {
        castlings_[static_cast<size_t>(Figure::Move::Castling::q)] = false;
      }
      break;
    }
    case Field::H: {
      if (number == Field::ONE) {
        castlings_[static_cast<size_t>(Figure::Move::Castling::K)] = false;
      } else {
        castlings_[static_cast<size_t>(Figure::Move::Castling::k)] = false;
      }
      break;
    }
    case Field::E: {
      if (number == Field::ONE) {
        castlings_[static_cast<size_t>(Figure::Move::Castling::K)] = false;
        castlings_[static_cast<size_t>(Figure::Move::Castling::Q)] = false;
      } else {
        castlings_[static_cast<size_t>(Figure::Move::Castling::k)] = false;
        castlings_[static_cast<size_t>(Figure::Move::Castling::q)] = false;
      }
      break;
    }
    default: {
      break;
    }
  }
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

  Figure::Color color = figure->getColor();

  std::unique_ptr<Figure> beaten_figure;
  if (fields_[move.new_field.letter][move.new_field.number] != nullptr) {
    beaten_figure = std::move(removeFigure(move.new_field));
    move.figure_beaten = true;
    if (rev_mode == false) {
      for (auto drawer : drawers_) {
        drawer->onFigureRemoved(move.new_field);
      }
    }
  }

  // Handle pawn promotion
  std::unique_ptr<Figure> promoted_pawn;
  if (move.pawn_promotion != Figure::PAWN) {
    assert(figure->getType() == Figure::PAWN);
    const Pawn* pawn = static_cast<const Pawn*>(figure);
    addFigure(move.pawn_promotion, move.new_field, pawn->getColor());
    promoted_pawn = std::move(removeFigure(move.old_field));
    if (rev_mode == false) {
      for (auto drawer : drawers_) {
        drawer->onFigureRemoved(move.old_field);
      }
    }
    figure = nullptr;
  }

  if (rev_mode == true) {
    ReversibleMove reversible_move(move,
                                   std::move(beaten_figure),
                                   std::move(promoted_pawn),
                                   en_passant_pawn_,
                                   castlings_);
    reversible_moves_.push_back(std::move(reversible_move));
  }

  // Handle en passant
  if (Figure::Move::isEnPassant(this, move.old_field, move.new_field) == true) {
    en_passant_pawn_ = static_cast<const Pawn*>(figure);
  } else {
    en_passant_pawn_ = nullptr;
  }

  // Handle castling
  if (move.castling != Figure::Move::Castling::LAST) {
    assert(figure->getType() == Figure::KING);
    Field::Letter old_l = move.castling == Figure::Move::Castling::K || move.castling == Figure::Move::Castling::k ? Field::H : Field::A;
    Field old_rook_position(old_l, move.old_field.number);
    Field::Letter new_l = move.castling == Figure::Move::Castling::K || move.castling == Figure::Move::Castling::k ? Field::F : Field::D;
    Field new_rook_position(new_l, move.old_field.number);
    moveFigure(old_rook_position, new_rook_position);
  }

  updateCastlings(move);


  if (figure != nullptr) {
    figure->setPosition(move.new_field);
    fields_[move.new_field.letter][move.new_field.number] = figure;
  }
  fields_[move.old_field.letter][move.old_field.number] = nullptr;

  if (rev_mode == false) {
    move.is_check = isKingChecked(!color);
    move.is_mate = isKingCheckmated(!color);
  
    for (auto drawer : drawers_) {
      drawer->onFigureMoved(move);
    }
  }

  GameStatus status = GameStatus::NONE;
  if (rev_mode == false) {
    status = getGameStatus(!color);
    if (status != GameStatus::NONE) {
      onGameFinished(status);
    }
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

  bool is_mate = true;
  auto figures = getFigures(color);
  for (const auto* figure: figures) {
    auto moves = figure->calculatePossibleMoves();
    for (const auto& move: moves) {
      auto wrapper = makeReversibleMove(move);
      if (isKingChecked(color) == false) {
        is_mate = false;
        break;
      }
    }
  }
  
  return is_mate;
}

bool Board::isKingStalemated(Figure::Color color) {
  const King* king = getKing(color);
  if (king == nullptr || isKingChecked(color) == true) {
    return false;
  }

  bool is_stalemate = true;
  auto figures = getFigures(color);
  for (const auto* figure: figures) {
    auto moves = figure->calculatePossibleMoves();
    for (const auto& move: moves) {
      auto wrapper = makeReversibleMove(move);
      if (isKingChecked(color) == false) {
        is_stalemate = false;
        break;
      }
    }
  }

  return is_stalemate;
}

Board::GameStatus Board::getGameStatus(Figure::Color color) {
  const King* king = getKing(Figure::WHITE);
  if (king == nullptr) {
    throw BadBoardStatusException(this);
  }
  king = getKing(Figure::BLACK);
  if (king == nullptr) {
    throw BadBoardStatusException(this);
  }
  Board::GameStatus status = isCheckMate();
  if (status == GameStatus::WHITE_WON ||
      status == GameStatus::BLACK_WON) {
    return status;
  }
  if (isKingStalemated(color) || isDraw()) {
    return GameStatus::DRAW;
  }
  if (isKingChecked(!color)) {
    throw BadBoardStatusException(this);
  }

  return GameStatus::NONE;
}

Board::GameStatus Board::isCheckMate() {
  if (isKingCheckmated(Figure::WHITE)) {
    return GameStatus::BLACK_WON;
  }
  if (isKingCheckmated(Figure::BLACK)) {
    return GameStatus::WHITE_WON;
  }
  return GameStatus::NONE;
}

bool Board::isDraw() const {
  const King* king = getKing(Figure::WHITE);
  if (king == nullptr) {
    return false;
  }
  king = getKing(Figure::BLACK);
  if (king == nullptr) {
    return false;
  }
  std::vector<const Figure*> white_figures = getFigures(Figure::WHITE);
  std::vector<const Figure*> black_figures = getFigures(Figure::BLACK);
  if (white_figures.size() > 1 || black_figures.size() > 1) {
    return false;
  }
  return true;
}

bool Board::canCastle(Figure::Move::Castling castling) const {
  assert(castling < Figure::Move::Castling::LAST);
  if(castlings_[static_cast<size_t>(castling)] == false) {
    return false;
  }
  Figure::Color color = (castling == Figure::Move::Castling::Q || castling == Figure::Move::Castling::K) ? Figure::WHITE : Figure::BLACK;
  const King* king = getKing(color);
  return king->canCastle(castling == Figure::Move::Castling::K || castling == Figure::Move::Castling::k);
}

Board::GameStatus Board::makeMove(Field old_field, Field new_field, Figure::Type promotion, bool rev_mode) {
  Figure* figure = fields_[old_field.letter][old_field.number];
  if (figure == nullptr) {
    throw NoFigureException(old_field);
  }

  if (isMoveValid(old_field, new_field) == false) {
    throw IllegalMoveException(figure, new_field);
  }

  Figure::Move::Castling castling = Figure::Move::isCastling(this, old_field, new_field);
  if (castling != Figure::Move::Castling::LAST) {
    if (figure->getType() != Figure::KING) {
      throw IllegalMoveException(figure, new_field);
    }
    const King* king = static_cast<const King*>(figure);
    if (king->canCastle(castling == Figure::Move::Castling::K || castling == Figure::Move::Castling::k) == false) {
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
  if (move.castling != Figure::Move::Castling::LAST) {
    if (castlings_[static_cast<size_t>(move.castling)] == false) {
      return false;
    }
    if (isKingChecked(color) == true) {
      return false;
    }
    const Field::Number number = color == Figure::WHITE ? Field::ONE : Field::EIGHT;
    const int offset = move.castling == Figure::Move::Castling::K || move.castling == Figure::Move::Castling::k ? 1 : -1;
    const Field new_field(static_cast<Field::Letter>(move.old_field.letter + offset), number);
    moveFigure(move.old_field, new_field);
    bool is_king_checked = isKingChecked(color);
    moveFigure(new_field, move.old_field);
    if (is_king_checked == true) {
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
  return true;
}

std::vector<Figure::Move> Board::calculateMovesForFigure(const Figure* figure) {
  std::vector<Figure::Move> moves = figure->calculatePossibleMoves();
  moves.erase(std::remove_if(moves.begin(), moves.end(),
      [this, figure](auto& move) -> bool {
        return isMoveValid(move, figure->getColor()) == false;
      }), moves.end());
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
  Figure* promoted_pawn = reversible_move.promoted_pawn.get();
  if (promoted_pawn != nullptr) {
    figures_.push_back(std::move(reversible_move.promoted_pawn));
    Field old_position = promoted_pawn->getPosition();
    fields_[old_position.letter][old_position.number] = promoted_pawn;
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
  en_passant_pawn_ = reversible_move.en_passant_pawn;
  castlings_ = reversible_move.castlings;
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
