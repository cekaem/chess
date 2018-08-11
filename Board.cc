#include "Board.h"

#include <algorithm>
#include <sstream>

#include "utils/Utils.h"


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

  castlings_ = other.castlings_;
  en_passant_file_ = other.en_passant_file_;
  halfmove_clock_ = other.halfmove_clock_;
  fullmove_number_ = other.fullmove_number_;
  side_to_move_ = other.side_to_move_;
}

bool Board::isMoveValid(Field old_field, Field new_field) {
  Figure* figure = fields_[old_field.letter][old_field.number];
  if (figure == nullptr) {
    return false;
  }

  if (figure->getColor() != side_to_move_) {
    return false;
  }

  try {
    GameStatus status = getGameStatus(figure->getColor());
    if (status != GameStatus::NONE) {
      return false;
    }
  } catch (const BadBoardStatusException&) {
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
  return en_passant_file_ == other.en_passant_file_ && castlings_ == other.castlings_ &&
         halfmove_clock_ == other.halfmove_clock_ && fullmove_number_ == other.fullmove_number_;
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

  BoardAssert(*this, iter != figures_.end());
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
  Figure::Color color = figure->getColor();

  if (color != side_to_move_ && rev_mode == false) {
    throw IllegalMoveException(figure, move.new_field);
  }

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

  // Handle en passant capture
  if (isEnPassantCapture(move) == true) {
    Field bitten_pawn_field;
    bitten_pawn_field.letter = en_passant_file_;
    if (color == Figure::WHITE) {
      bitten_pawn_field.number = Field::FIVE;
    } else {
      bitten_pawn_field.number = Field::FOUR;
    }
    beaten_figure = std::move(removeFigure(bitten_pawn_field));
  }

  // Handle pawn promotion
  std::unique_ptr<Figure> promoted_pawn;
  if (move.pawn_promotion != Figure::PAWN) {
    BoardAssert(*this, figure->getType() == Figure::PAWN);
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
                                   en_passant_file_,
                                   castlings_,
                                   halfmove_clock_,
                                   fullmove_number_,
                                   side_to_move_);
    reversible_moves_.push_back(std::move(reversible_move));
  }

  side_to_move_ = !side_to_move_;

  if (color == Figure::BLACK) {
    ++fullmove_number_;
  }

  // Update en_passant_file_
  if (Figure::Move::isTwoSquaresPawnMove(this, move.old_field, move.new_field) == true) {
    en_passant_file_ = move.old_field.letter;
  } else {
    en_passant_file_ = Field::Letter::NONE;
  }

  // Handle castling
  if (move.castling != Figure::Move::Castling::LAST) {
    BoardAssert(*this, figure->getType() == Figure::KING);
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

  if (figure != nullptr && figure->getType() != Figure::PAWN && move.figure_beaten == false) {
    ++halfmove_clock_;
  } else {
    halfmove_clock_ = 0;
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

bool Board::isEnPassantCapture(const Figure::Move& move) const {
  const Figure* figure = getFigure(move.old_field);
  if (figure == nullptr || figure->getType() != Figure::PAWN) {
    return false;
  }
  if (move.new_field.letter != en_passant_file_) {
    return false;
  }
  if (figure->getColor() == Figure::WHITE) {
    if (move.new_field.number != Field::SIX) {
      return false;
    }
  } else {
    if (move.new_field.number != Field::THREE) {
      return false;
    }
  }
  return true;
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
      if (move.castling != Figure::Move::Castling::LAST) {
        continue;  // You can't castle when you're in check
      }
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

bool Board::canKingCastle(Figure::Color color) const {
  if (color == Figure::WHITE) {
    return castlings_[static_cast<size_t>(Figure::Move::Castling::Q)] == true ||
           castlings_[static_cast<size_t>(Figure::Move::Castling::K)] == true;
  }
  return castlings_[static_cast<size_t>(Figure::Move::Castling::q)] == true ||
         castlings_[static_cast<size_t>(Figure::Move::Castling::k)] == true;
}

std::string Board::createFEN() const {
  std::stringstream fen;

  for (int j = BoardSize - 1; j >= 0; --j) {
    int number_of_empty_fields = 0;
    for (int i = 0; i < static_cast<int>(BoardSize); ++i) {
      const Figure* figure = fields_[i][j];
      if (figure == nullptr) {
        ++number_of_empty_fields;
      } else {
        if (number_of_empty_fields != 0) {
          fen << number_of_empty_fields;
          number_of_empty_fields = 0;
        }
        fen << figure->getFENNotation();
      }
    }
    if (number_of_empty_fields != 0) {
      fen << number_of_empty_fields;
    }
    if (j != 0) {
      fen << '/';
    }
  }
  fen << ' ';

  fen << (side_to_move_ == Figure::WHITE ? 'w' : 'b');
  fen << ' ';

  std::stringstream castlings;
  if (castlings_[static_cast<size_t>(Figure::Move::Castling::K)] == true) {
    castlings << 'K';
  }
  if (castlings_[static_cast<size_t>(Figure::Move::Castling::Q)] == true) {
    castlings << 'Q';
  }
  if (castlings_[static_cast<size_t>(Figure::Move::Castling::k)] == true) {
    castlings << 'k';
  }
  if (castlings_[static_cast<size_t>(Figure::Move::Castling::q)] == true) {
    castlings << 'q';
  }
  if (castlings.str().empty() == true) {
    fen << '-';
  } else {
    fen << castlings.str();
  }
  fen << ' ';

  if (en_passant_file_ == Field::NONE) {
    fen << '-';
  } else {
    fen << static_cast<char>(en_passant_file_ + 'a');
    fen << (side_to_move_ == Figure::WHITE ? 6 : 3);
  }
  fen << ' ' << (halfmove_clock_ / 2) << ' ' << fullmove_number_;

  return fen.str();
}

bool Board::setBoardFromFEN(const std::string& fen) {
  clearBoard();
  std::string::size_type space_position = fen.find(' ');
  if (space_position == std::string::npos) {
    return false;
  }
  std::string fields = fen.substr(0, space_position);
  std::string rest_fen = fen.substr(space_position);
  if (rest_fen.size() < 10) {
    // 10 is the minimal valid length of the rest of the fen " w - - 0 0"
    return false;
  }
  if (rest_fen[0] != ' ' || rest_fen[2] != ' ') {
    return false;
  }
  for (int i = BoardSize - 1; i >= 0; --i) {
    std::string partial_fen;
    if (i > 0) {
      std::string::size_type slash_position = fields.find('/');
      if (slash_position == std::string::npos) {
        return false;
      }
      partial_fen = fields.substr(0, slash_position);
      fields = fields.substr(slash_position + 1);
    } else {
      partial_fen = fields;
    }
    if (addFiguresForOneLineFromFen(partial_fen, i) == false) {
      return false;
    }
  }
  
  const char side_to_move = rest_fen[1];
  switch (side_to_move) {
    case 'w':
      side_to_move_ = Figure::WHITE;
      break;
    case 'b':
      side_to_move_ = Figure::BLACK;
      break;
    default:
      return false;
  }

  rest_fen = rest_fen.substr(3);
  space_position = rest_fen.find(' ');
  if (space_position == std::string::npos) {
    return false;
  }
  std::string castlings = rest_fen.substr(0, space_position);
  if (setCastlingsFromFen(castlings) == false) {
    return false;
  }
  rest_fen = rest_fen.substr(space_position);
  if (rest_fen.size() < 6 || rest_fen[0] != ' ') {  // 6 == length(" - 0 0")
    return false;
  }
  rest_fen = rest_fen.substr(1);
  space_position = rest_fen.find(' ');
  if (space_position == std::string::npos) {
    return false;
  }
  std::string en_passant_file = rest_fen.substr(0, space_position);
  if (setEnPassantFileFromFen(en_passant_file, side_to_move_ == Figure::WHITE) == false) {
    return false;
  }
  rest_fen = rest_fen.substr(space_position);
  if (rest_fen.size() < 4 || rest_fen[0] != ' ') {  // 4 == length(" 0 0")
    return false;
  }
  rest_fen = rest_fen.substr(1);
  space_position = rest_fen.find(' ');
  if (space_position == std::string::npos) {
    return false;
  }
  std::string halfmove_clock_str = rest_fen.substr(0, space_position);
  if (utils::str_2_uint(halfmove_clock_str, halfmove_clock_) == false) {
    return false;
  }
  halfmove_clock_ *= 2;
  rest_fen = rest_fen.substr(space_position);
  if (rest_fen.size() < 2 || rest_fen[0] != ' ') {  // 2 == length(" 0")
    return false;
  }
  std::string fullmove_number_str = rest_fen.substr(1);
  if (utils::str_2_uint(fullmove_number_str, fullmove_number_) == false) {
    return false;
  }

  return true;
}

bool Board::setEnPassantFileFromFen(const std::string& fen, bool white_to_move) {
  if (fen.size() == 1) {
    return fen[0] == '-';
  }
  if (Field::isFieldValid(fen) == false) {
    return false;
  }
  Field field(fen.c_str());
  if ((white_to_move == true && field.number != Field::SIX) ||
      (white_to_move == false && field.number != Field::THREE)) {
    return false;
  }
  en_passant_file_ = field.letter;
  return true;
}

bool Board::setCastlingsFromFen(const std::string& fen) {
  if (fen.empty() == true || fen.size() > 4) {
    return false;
  }
  castlings_[static_cast<size_t>(Figure::Move::Castling::K)] = false;
  castlings_[static_cast<size_t>(Figure::Move::Castling::k)] = false;
  castlings_[static_cast<size_t>(Figure::Move::Castling::Q)] = false;
  castlings_[static_cast<size_t>(Figure::Move::Castling::q)] = false;
  for (const char c: fen) {
    switch (c) {
      case 'K':
        castlings_[static_cast<size_t>(Figure::Move::Castling::K)] = true;
        break;
      case 'k':
        castlings_[static_cast<size_t>(Figure::Move::Castling::k)] = true;
        break;
      case 'Q':
        castlings_[static_cast<size_t>(Figure::Move::Castling::Q)] = true;
        break;
      case 'q':
        castlings_[static_cast<size_t>(Figure::Move::Castling::q)] = true;
        break;
      case '-': 
        if (fen.size() != 1) {
          return false;
        }
        break;
      default:
        return false;
    }
  }
  return true;
}

bool Board::addFigure(const char fen_char, Field field) {
  Figure::Type type;
  Figure::Color color;
  switch (fen_char) {
    case 'p':
      type = Figure::PAWN;
      color = Figure::BLACK;
      break;
    case 'P':
      type = Figure::PAWN;
      color = Figure::WHITE;
      break;
    case 'n':
      type = Figure::KNIGHT;
      color = Figure::BLACK;
      break;
    case 'N':
      type = Figure::KNIGHT;
      color = Figure::WHITE;
      break;
    case 'b':
      type = Figure::BISHOP;
      color = Figure::BLACK;
      break;
    case 'B':
      type = Figure::BISHOP;
      color = Figure::WHITE;
      break;
    case 'r':
      type = Figure::ROOK;
      color = Figure::BLACK;
      break;
    case 'R':
      type = Figure::ROOK;
      color = Figure::WHITE;
      break;
    case 'q':
      type = Figure::QUEEN;
      color = Figure::BLACK;
      break;
    case 'Q':
      type = Figure::QUEEN;
      color = Figure::WHITE;
      break;
    case 'k':
      type = Figure::KING;
      color = Figure::BLACK;
      break;
    case 'K':
      type = Figure::KING;
      color = Figure::WHITE;
      break;
    default:
      return false;
  }
  addFigure(type, field, color);
  return true;
}

bool Board::addFiguresForOneLineFromFen(const std::string& fen, size_t line) {
  size_t current_file = 0;
  for (const char c: fen) {
    if (current_file >= BoardSize) {
      return false;
    }
    if (c >= '1' && c <= '8') {
      current_file += c - '0';
    } else {
      if (addFigure(c, Field(static_cast<Field::Letter>(current_file),
                             static_cast<Field::Number>(line))) == false) {
        return false;
      }
      ++current_file;
    }
  }
  return true;
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
  if (halfmove_clock_ >= 100) {
    return true;
  }
  std::vector<const Figure*> white_figures = getFigures(Figure::WHITE);
  std::vector<const Figure*> black_figures = getFigures(Figure::BLACK);
  if (white_figures.size() > 1 || black_figures.size() > 1) {
    return false;
  }
  return true;
}

bool Board::canCastle(Figure::Move::Castling castling) const {
  BoardAssert(*this, castling < Figure::Move::Castling::LAST);
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

std::vector<Figure::Move> Board::calculateMovesForFigures(Figure::Color color) {
  std::vector<Figure::Move> all_moves;
  auto figures = getFigures(color);
  for (const auto* figure: figures) {
    auto moves = calculateMovesForFigure(figure);
    all_moves.insert(all_moves.end(), moves.begin(), moves.end());
  }
  return all_moves;
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

std::string Board::getFENForCastlings() const noexcept {
  std::string fen;
  if (castlings_[static_cast<size_t>(Figure::Move::Castling::K)] == true) {
    fen.push_back('K');
  }
  if (castlings_[static_cast<size_t>(Figure::Move::Castling::Q)] == true) {
    fen.push_back('Q');
  }
  if (castlings_[static_cast<size_t>(Figure::Move::Castling::k)] == true) {
    fen.push_back('k');
  }
  if (castlings_[static_cast<size_t>(Figure::Move::Castling::q)] == true) {
    fen.push_back('q');
  }
  if (fen.size() == 0) {
    fen.push_back('-');
  }
  return fen;
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
  BoardAssert(*this, reversible_moves_.empty() == false);
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
  en_passant_file_ = reversible_move.en_passant_file;
  castlings_ = reversible_move.castlings;
  halfmove_clock_ = reversible_move.halfmove_clock;
  fullmove_number_ = reversible_move.fullmove_number;
  side_to_move_ = reversible_move.side_to_move;
}

void Board::undoAllReversibleMoves() {
  while (reversible_moves_.empty() == false) {
    undoLastReversibleMove();
  }
}

void Board::clearBoard() {
  for (size_t i = 0; i < BoardSize; ++i) {
    for (size_t j = 0; j < BoardSize; ++j) {
      if (fields_[i][j] != nullptr) {
        Field field(static_cast<Field::Letter>(i), static_cast<Field::Number>(j));
        removeFigure(field);
        for (auto drawer : drawers_) {
          drawer->onFigureRemoved(field);
        }
      }
    }
  }
  castlings_[static_cast<size_t>(Figure::Move::Castling::Q)] = true;
  castlings_[static_cast<size_t>(Figure::Move::Castling::K)] = true;
  castlings_[static_cast<size_t>(Figure::Move::Castling::q)] = true;
  castlings_[static_cast<size_t>(Figure::Move::Castling::k)] = true;
  en_passant_file_ = Field::NONE;
  reversible_moves_.clear();
}

void Board::setStandardBoard() {
  setBoardFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
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
