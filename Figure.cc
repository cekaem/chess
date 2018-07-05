#include "Figure.h"

#include <algorithm>
#include <cassert>

#include "Board.h"

namespace {

void addMove(const Board& board,
             std::vector<Figure::Move>& moves,
             const Figure* figure,
             unsigned new_l,
             unsigned new_n,
             Figure::Type promo = Figure::PAWN) {
  Figure::Move move(
      figure->getPosition(),
      Field(static_cast<Field::Letter>(new_l), static_cast<Field::Number>(new_n)),
      false,  // it will be updated later
      false,  // it will be updated later
      Figure::Move::Castling::NONE,
      board.getFigure(Field(static_cast<Field::Letter>(new_l), static_cast<Field::Number>(new_n))) != nullptr,
      promo);
  moves.push_back(move);
}

void calculateMovesForBishop(std::vector<Figure::Move>& moves, const Board& board, Field field) {
  const Figure* bishop = board.getFigure(field);
  Figure::Color my_color = bishop->getColor();

  int l = field.letter - 1;
  int n = field.number - 1;
  while (l >= Field::A && n >= Field::ONE) {
    const Figure* figure = board.getFigure(Field(static_cast<Field::Letter>(l),
                                                 static_cast<Field::Number>(n)));
    if (!figure || (figure && figure->getColor() != my_color)) {
      addMove(board, moves, bishop, l, n);
    }
    if (figure != nullptr) {
      break;
    }
    --l, --n;
  }

  l = field.letter - 1;
  n = field.number + 1;
  while (l >= Field::A && n <= Field::EIGHT) {
    const Figure* figure = board.getFigure(Field(static_cast<Field::Letter>(l),
                                                 static_cast<Field::Number>(n)));
    if (!figure || (figure && figure->getColor() != my_color)) {
      addMove(board, moves, bishop, l, n);
    }
    if (figure != nullptr) {
      break;
    }
    --l, ++n;
  }

  l = field.letter + 1;
  n = field.number + 1;
  while (l <= Field::H && n <= Field::EIGHT) {
    const Figure* figure = board.getFigure(Field(static_cast<Field::Letter>(l),
                                                 static_cast<Field::Number>(n)));
    if (!figure || (figure && figure->getColor() != my_color)) {
      addMove(board, moves, bishop, l, n);
    }
    if (figure != nullptr) {
      break;
    }
    ++l, ++n;
  }

  l = field.letter + 1;
  n = field.number - 1;
  while (l <= Field::H && n >= Field::ONE) {
    const Figure* figure = board.getFigure(Field(static_cast<Field::Letter>(l),
                                                 static_cast<Field::Number>(n)));
    if (!figure || (figure && figure->getColor() != my_color)) {
      addMove(board, moves, bishop, l, n);
    }
    if (figure != nullptr) {
      break;
    }
    ++l, --n;
  }
}

void calculateMovesForRook(std::vector<Figure::Move>& moves, const Board& board, Field field) {
  const Figure* rook = board.getFigure(field);
  Figure::Color my_color = rook->getColor();

  int l = field.letter - 1;
  int n = field.number;
  while (l >= Field::A) {
    const Figure* figure = board.getFigure(Field(static_cast<Field::Letter>(l),
                                                 static_cast<Field::Number>(n)));
    if (!figure || (figure && figure->getColor() != my_color)) {
      addMove(board, moves, rook, l, n);
    }
    if (figure != nullptr) {
      break;
    }
    --l;
  }

  l = field.letter + 1;
  while (l <= Field::H) {
    const Figure* figure = board.getFigure(Field(static_cast<Field::Letter>(l),
                                                 static_cast<Field::Number>(n)));
    if (!figure || (figure && figure->getColor() != my_color)) {
      addMove(board, moves, rook, l, n);
    }
    if (figure != nullptr) {
      break;
    }
    ++l;
  }

  l = field.letter;
  n = field.number + 1;
  while (n <= Field::EIGHT) {
    const Figure* figure = board.getFigure(Field(static_cast<Field::Letter>(l),
                                                 static_cast<Field::Number>(n)));
    if (!figure || (figure && figure->getColor() != my_color)) {
      addMove(board, moves, rook, l, n);
    }
    if (figure != nullptr) {
      break;
    }
    ++n;
  }

  n = field.number - 1;
  while (n >= Field::ONE) {
    const Figure* figure = board.getFigure(Field(static_cast<Field::Letter>(l),
                                                 static_cast<Field::Number>(n)));
    if (!figure || (figure && figure->getColor() != my_color)) {
      addMove(board, moves, rook, l, n);
    }
    if (figure != nullptr) {
      break;
    }
    --n;
  }
}

}  // unnamed namespace

Figure::Move::Castling Figure::Move::isCastling(const Board* board, Field old_field, Field new_field) {
  const Figure* figure = board->getFigure(old_field);
  if (figure == nullptr || figure->getType() != Figure::KING) {
    return Figure::Move::Castling::NONE;
  }
  Field::Number number = figure->getColor() == Figure::WHITE ? Field::ONE : Field::EIGHT;
  if (old_field != Field(Field::E, number)) {
    return Figure::Move::Castling::NONE;
  }
  if (new_field == Field(Field::G, number)) {
    return Figure::Move::Castling::KING_SIDE;
  }
  if (new_field == Field(Field::C, number)) {
    return Figure::Move::Castling::QUEEN_SIDE;
  }
  return Figure::Move::Castling::NONE;
}

bool Figure::Move::isPromotion(const Board* board, Field old_field, Field new_field) {
  const Figure* figure = board->getFigure(old_field);
  if (figure == nullptr || figure->getType() != Figure::PAWN) {
    return false;
  }
  Field::Number old_number = figure->getColor() == Figure::WHITE ? Field::SEVEN : Field::TWO;
  Field::Number new_number = figure->getColor() == Figure::WHITE ? Field::EIGHT : Field::ONE;
  return old_field.number == old_number && new_field.number == new_number;
}

bool Figure::Move::operator==(const Figure::Move& other) const {
  return old_field == other.old_field &&
         new_field == other.new_field &&
         is_check == other.is_check &&
         is_mate == other.is_mate &&
         castling == other.castling &&
         figure_beaten == other.figure_beaten &&
         pawn_promotion == other.pawn_promotion;
}

std::ostream& operator<<(std::ostream& ostr, const Figure::Move& move) {
  ostr << move.old_field << "-" << move.new_field << "(" << move.is_check << ", " << move.is_mate << ", ";
  switch (move.castling) {
    case Figure::Move::Castling::NONE:
      ostr << "NONE";
      break;
    case Figure::Move::Castling::QUEEN_SIDE:
      ostr << "Castling::QUEEN_SIDE";
      break;
    case Figure::Move::Castling::KING_SIDE:
      ostr << "Castling::KING_SIDE";
      break;
  }
  ostr << ", " << move.figure_beaten << ", " << move.pawn_promotion << ")";
  return ostr;
}

FiguresFactory& FiguresFactory::GetFiguresFactory() noexcept {
  static FiguresFactory factory;
  return factory;
}

FiguresFactory::FiguresFactory() noexcept {
  // TODO: add some cache for figures
}

std::unique_ptr<Figure> FiguresFactory::createFigure(
    Figure::Type type, Board& board, Field field, Figure::Color color) noexcept {
  switch (type) {
    case Figure::PAWN:
      return std::make_unique<Pawn>(board, field, color);
    case Figure::KNIGHT:
      return std::make_unique<Knight>(board, field, color);
    case Figure::BISHOP:
      return std::make_unique<Bishop>(board, field, color);
    case Figure::ROOK:
      return std::make_unique<Rook>(board, field, color);
    case Figure::QUEEN:
      return std::make_unique<Queen>(board, field, color);
    case Figure::KING:
      return std::make_unique<King>(board, field, color);
  }
  assert(!"It should never reached this point.");
}

Figure::Figure(Board& board, Field field, Color color, int value) noexcept
  : board_(board), field_(field), color_(color), value_(value) {
}

void Figure::move(Figure::Move move) {
  field_ = move.new_field;
  moved_at_least_once_ = true;
}

bool Figure::operator==(const Figure& other) const {
  return getType() == other.getType() && getColor() == other.getColor();
}

bool Figure::operator!=(const Figure& other) const {
  return !(*this == other);
}

bool Pawn::canPromote() const {
  return (getColor() == WHITE && field_.number == Field::SEVEN) ||
         (getColor() == BLACK && field_.number == Field::TWO);
}

std::vector<Figure::Move> Pawn::calculatePossibleMoves() const {
  std::vector<Move> result;
  Field::Letter current_l = field_.letter;
  Field::Number current_n = field_.number;

  const auto& fields = board_.getFields();
  const int offset = getColor() == WHITE ? 1 : -1;

  if (fields[current_l][current_n + offset] == nullptr) {
    if (canPromote()) {
      addMove(board_, result, this, current_l, current_n + offset, Figure::BISHOP);
      addMove(board_, result, this, current_l, current_n + offset, Figure::KNIGHT);
      addMove(board_, result, this, current_l, current_n + offset, Figure::ROOK);
      addMove(board_, result, this, current_l, current_n + offset, Figure::QUEEN);
    } else {
      addMove(board_, result, this, current_l, current_n + offset);
    }
  }

  if ((getColor() == WHITE && field_.number == Field::TWO) ||
      (getColor() == BLACK && field_.number == Field::SEVEN)) {
    if (fields[current_l][current_n + offset] == nullptr &&
        fields[current_l][current_n + 2 * offset] == nullptr) {
      addMove(board_, result, this, current_l, current_n + 2 * offset);
    }
  }

  if (current_l != Field::A) {
    const Figure* figure = fields[current_l - 1][current_n + offset];
    if (figure != nullptr && figure->getColor() != getColor()) {
      if (canPromote()) {
        addMove(board_, result, this, current_l - 1, current_n + offset, Figure::BISHOP);
        addMove(board_, result, this, current_l - 1, current_n + offset, Figure::KNIGHT);
        addMove(board_, result, this, current_l - 1, current_n + offset, Figure::ROOK);
        addMove(board_, result, this, current_l - 1, current_n + offset, Figure::QUEEN);
      } else {
        addMove(board_, result, this, current_l - 1, current_n + offset);
      }
    }
  }

  if (current_l != Field::H) {
    const Figure* figure = fields[current_l + 1][current_n + offset];
    if (figure != nullptr && figure->getColor() != getColor()) {
      if (canPromote()) {
        addMove(board_, result, this, current_l + 1, current_n + offset, Figure::BISHOP);
        addMove(board_, result, this, current_l + 1, current_n + offset, Figure::KNIGHT);
        addMove(board_, result, this, current_l + 1, current_n + offset, Figure::ROOK);
        addMove(board_, result, this, current_l + 1, current_n + offset, Figure::QUEEN);
      } else {
        addMove(board_, result, this, current_l + 1, current_n + offset);
      }
    }
  }

  // Check for "en passant"
  const Pawn* en_passant = board_.getEnPassantPawn();
  if (en_passant != nullptr) {
    if ((getColor() == WHITE && field_.number == Field::FIVE) ||
        (getColor() == BLACK && field_.number == Field::FOUR)) {
      if (field_.letter != Field::A && fields[field_.letter - 1][field_.number] == en_passant) {
        addMove(board_, result, this, current_l - 1, current_n + offset);
        result[result.size() - 1].figure_beaten = true;
      } else if (field_.letter != Field::H && fields[field_.letter + 1][field_.number] == en_passant) {
        addMove(board_, result, this, current_l + 1, current_n + offset);
        result[result.size() - 1].figure_beaten = true;
      }
    }
  }

  return result;
}

std::vector<Figure::Move> Knight::calculatePossibleMoves() const {
  std::vector<Move> result;
  Field::Letter current_l = field_.letter;
  Field::Number current_n = field_.number;

  std::vector<std::pair<int, int>> possible_moves;
  possible_moves.push_back(std::make_pair(current_l + 2, current_n - 1));
  possible_moves.push_back(std::make_pair(current_l + 2, current_n + 1));
  possible_moves.push_back(std::make_pair(current_l + 1, current_n + 2));
  possible_moves.push_back(std::make_pair(current_l - 1, current_n + 2));
  possible_moves.push_back(std::make_pair(current_l - 2, current_n - 1));
  possible_moves.push_back(std::make_pair(current_l - 2, current_n + 1));
  possible_moves.push_back(std::make_pair(current_l + 1, current_n - 2));
  possible_moves.push_back(std::make_pair(current_l - 1, current_n - 2));

  possible_moves.erase(std::remove_if(possible_moves.begin(), possible_moves.end(),
                 [](const auto& iter) -> bool {
                   return iter.first < Field::A || iter.first > Field::H ||
                          iter.second < Field::ONE || iter.second > Field::EIGHT;
                 }), possible_moves.end());

  for (const auto& iter : possible_moves) {
    Field field(static_cast<Field::Letter>(iter.first),
                static_cast<Field::Number>(iter.second));
    const Figure* figure = board_.getFigure(field);
    if (figure == nullptr || (figure != nullptr  && figure->getColor() != getColor())) {
      addMove(board_, result, this, field.letter, field.number);
    }
  }

  return result;
}

std::vector<Figure::Move> Bishop::calculatePossibleMoves() const {
  std::vector<Move> result;
  calculateMovesForBishop(result, board_, field_);
  return result;
}

std::vector<Figure::Move> Rook::calculatePossibleMoves() const {
  std::vector<Move> result;
  calculateMovesForRook(result, board_, field_);
  return result;
}

std::vector<Figure::Move> Queen::calculatePossibleMoves() const {
  std::vector<Move> result;
  calculateMovesForBishop(result, board_, field_);
  calculateMovesForRook(result, board_, field_);
  return result;
}

std::vector<Figure::Move> King::calculatePossibleMoves() const {
  std::vector<Move> result;
  Field::Letter current_l = field_.letter;
  Field::Number current_n = field_.number;

  std::vector<std::pair<int, int>> possible_moves;
  possible_moves.push_back(std::make_pair(current_l - 1, current_n));
  possible_moves.push_back(std::make_pair(current_l - 1, current_n + 1));
  possible_moves.push_back(std::make_pair(current_l, current_n + 1));
  possible_moves.push_back(std::make_pair(current_l + 1, current_n + 1));
  possible_moves.push_back(std::make_pair(current_l + 1, current_n));
  possible_moves.push_back(std::make_pair(current_l + 1, current_n - 1));
  possible_moves.push_back(std::make_pair(current_l, current_n - 1));
  possible_moves.push_back(std::make_pair(current_l - 1, current_n - 1));

  possible_moves.erase(std::remove_if(possible_moves.begin(), possible_moves.end(),
                 [this](const auto& iter) -> bool {
                   if (iter.first < Field::A || iter.first > Field::H ||
                       iter.second < Field::ONE || iter.second > Field::EIGHT) {
                     return true;
                   }
                   const Figure* figure = board_.getFigure(Field(
                         static_cast<Field::Letter>(iter.first),
                         static_cast<Field::Number>(iter.second)));
                   if (figure != nullptr && figure->getColor() == this->getColor()) {
                     return true;
                   }
                   return false;
                 }), possible_moves.end());

  // King can't go to field threated by enemy figure
  for (auto move : possible_moves) {
    addMove(board_, result, this, move.first, move.second);
  }
  if (looksForKingUnveils()) {
    addPossibleCastlings(result);
  }

  return result;
}

bool King::canCastle(Figure::Move::Castling castling) const {
  if (castling == Figure::Move::Castling::NONE) {
    return true;
  }

  if (movedAtLeastOnce()) {
    return false;
  }

  const Field::Number number = getColor() == Figure::WHITE ? Field::ONE : Field::EIGHT;

  // Check if king is in the right position
  if (getPosition() != Field(Field::E, number)) {
    return false;
  }

  if (board_.isKingChecked(getColor())) {
    return false;
  }

  bool result = false;
  if (castling == Figure::Move::Castling::KING_SIDE) {
    const Figure* figure = board_.getFigure(Field(Field::H, number));
    if (figure && figure->getType() == Figure::ROOK && figure->getColor() == getColor() &&
        figure->movedAtLeastOnce() == false && board_.getFigure(Field(Field::F, number)) == nullptr &&
        board_.getFigure(Field(Field::G, number)) == nullptr) {
      Board copy = board_;
      const Figure* f = copy.getFigure(Field(Field::E, number));
      assert(f->getType() == Figure::KING);
      const King* king = static_cast<const King*>(f)i;
      copy.makeMove(Field(Field::E, number), Field(Field::F, number), Figure::PAWN, true);
      if (board_.isKingChecked(getColor()) == false) {
        copy.makeMove(Field(Field::F, number), Field(Field::G, number), Figure::PAWN, true);
        if (board_.isKingChecked(getColor()) == false) {
          result = true;
        }
        board_.undoLastReversibleMove();
      }
      board_.undoLastReversibleMove();
    }
  }

  if (castling == Figure::Move::Castling::QUEEN_SIDE) {
    const Figure* figure = board_.getFigure(Field(Field::A, number));
    if (figure && figure->getType() == Figure::ROOK && figure->getColor() == getColor() &&
        figure->movedAtLeastOnce() == false && board_.getFigure(Field(Field::D, number)) == nullptr &&
        board_.getFigure(Field(Field::C, number)) == nullptr &&
        board_.getFigure(Field(Field::B, number)) == nullptr) {
      Board copy = board_;
      const Figure* f = copy.getFigure(Field(Field::E, number));
      assert(f->getType() == Figure::KING);
      const King* king = static_cast<const King*>(f);
      copy.makeMove(Field(Field::E, number), Field(Field::D, number), Figure::PAWN, true);
      if (board_.isKingChecked(getColor()) == false) {
        copy.makeMove(Field(Field::D, number), Field(Field::C, number), Figure::PAWN, true);
        if (board_.isKingChecked(getColor()) == false) {
          result = true;
        }
        board_.undoLastReversibleMove();
      }
      board_.undoLastReversibleMove();
    }
  }
  return result;
}

void King::addPossibleCastlings(std::vector<Move>& moves) const {
  const Field::Number number = getColor() == Figure::WHITE ? Field::ONE : Field::EIGHT;

  if (canCastle(Figure::Move::Castling::KING_SIDE) == true) {
    moves.push_back(Figure::Move(Field(Field::E, number),
                                 Field(Field::G, number),
                                 Figure::Move::Castling::KING_SIDE));
  }

  if (canCastle(Figure::Move::Castling::QUEEN_SIDE) == true) {
        moves.push_back(Figure::Move(Field(Field::E, number),
                                     Field(Field::C, number),
                                     Figure::Move::Castling::QUEEN_SIDE));
  }
}
