#include "Figure.h"

#include <algorithm>
#include <cassert>

#include "Board.h"

namespace {

void addMove(std::vector<Figure::Move>& moves, unsigned l, unsigned n, const Figure* f) {
  auto move = std::make_pair(Field(static_cast<Field::Letter>(l),
                                   static_cast<Field::Number>(n)),
                             f);
  moves.push_back(move);
}

void calculateMovesForBishop(std::vector<Figure::Move>& moves, const Board& board, Field field) {
  Figure::Color my_color = board.getFigure(field)->getColor();

  int l = field.letter - 1;
  int n = field.number - 1;
  while (l >= Field::A && n >= Field::ONE) {
    const Figure* figure = board.getFigure(Field(static_cast<Field::Letter>(l),
                                                 static_cast<Field::Number>(n)));
    if (!figure || (figure && figure->getColor() != my_color)) {
      addMove(moves, l, n, figure);
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
      addMove(moves, l, n, figure);
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
      addMove(moves, l, n, figure);
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
      addMove(moves, l, n, figure);
    }
    if (figure != nullptr) {
      break;
    }
    ++l, --n;
  }
}

void calculateMovesForRook(std::vector<Figure::Move>& moves, const Board& board, Field field) {
  Figure::Color my_color = board.getFigure(field)->getColor();

  int l = field.letter - 1;
  int n = field.number;
  while (l >= Field::A) {
    const Figure* figure = board.getFigure(Field(static_cast<Field::Letter>(l),
                                                 static_cast<Field::Number>(n)));
    if (!figure || (figure && figure->getColor() != my_color)) {
      addMove(moves, l, n, figure);
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
      addMove(moves, l, n, figure);
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
      addMove(moves, l, n, figure);
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
      addMove(moves, l, n, figure);
    }
    if (figure != nullptr) {
      break;
    }
    --n;
  }
}

void removeKingUnveils(std::vector<Figure::Move>& moves, const Board& board, const Figure* figure) {
  if (figure->looksForKingUnveils() == false) {
    return;
  }
  if (board.getKing(figure->getColor()) == nullptr) {
    return;
  }
  Field field = figure->getPosition();
  moves.erase(std::remove_if(moves.begin(), moves.end(),
        [board, field](const auto& move) -> bool {
          Board copy = board;
          const Figure* f = copy.getFigure(field);
          copy.moveFigure(field, move.first);
          return static_cast<const King*>(copy.getKing(f->getColor()))->isChecked();
        }), moves.end());
}

}  // unnamed namespace

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

void Figure::move(Field field) {
  field_ = field;
  moved_at_least_once_ = true;
  board_.setEnPassantPawn(nullptr);
}

bool Figure::operator==(const Figure& other) const {
  return getType() == other.getType() && getColor() == other.getColor();
}

bool Figure::operator!=(const Figure& other) const {
  return !(*this == other);
}

void Pawn::move(Field field) {
  Field old_field = field_;
  Figure::move(field);
  if ((getColor() == WHITE && old_field.number == Field::TWO && field.number == Field::FOUR) ||
      (getColor() == BLACK && old_field.number == Field::SEVEN && field.number == Field::FIVE)) {
    board_.setEnPassantPawn(this);
  }
}

std::vector<Figure::Move> Pawn::calculatePossibleMoves() const {
  std::vector<Move> result;
  Field::Letter current_l = field_.letter;
  Field::Number current_n = field_.number;

  if ((getColor() == WHITE && current_n == Field::EIGHT) ||
      (getColor() == BLACK && current_n == Field::ONE)) {
    return result;
  }

  const auto& fields = board_.getFields();
  const int offset = getColor() == WHITE ? 1 : -1;

  if (fields[current_l][current_n + offset] == nullptr) {
    addMove(result, current_l, current_n + offset, nullptr);
  }

  if ((getColor() == WHITE && field_.number == Field::TWO) ||
      (getColor() == BLACK && field_.number == Field::SEVEN)) {
    if (fields[current_l][current_n + offset] == nullptr &&
        fields[current_l][current_n + 2 * offset] == nullptr) {
      addMove(result, current_l, current_n + 2 * offset, nullptr);
    }
  }

  if (current_l != Field::A) {
    const Figure* figure = fields[current_l - 1][current_n + offset];
    if (figure != nullptr && figure->getColor() != getColor()) {
      addMove(result, current_l - 1, current_n + offset, figure);
    }
  }

  if (current_l != Field::H) {
    const Figure* figure = fields[current_l + 1][current_n + offset];
    if (figure != nullptr && figure->getColor() != getColor()) {
      addMove(result, current_l + 1, current_n + offset, figure);
    }
  }

  // Check for "en passant"
  const Figure* en_passant = board_.getEnPassantPawn();
  if (en_passant != nullptr) {
    if ((getColor() == WHITE && field_.number == Field::FIVE) ||
        (getColor() == BLACK && field_.number == Field::FOUR)) {
      if (field_.letter != Field::A && fields[field_.letter - 1][field_.number] == en_passant) {
        addMove(result, current_l - 1, current_n + offset, en_passant);
      } else if (field_.letter != Field::H && fields[field_.letter + 1][field_.number] == en_passant) {
        addMove(result, current_l + 1, current_n + offset, en_passant);
      }
    }
  }

  removeKingUnveils(result, board_, this);
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
      addMove(result, field.letter, field.number, figure);
    }
  }

  removeKingUnveils(result, board_, this);
  return result;
}

std::vector<Figure::Move> Bishop::calculatePossibleMoves() const {
  std::vector<Move> result;
  calculateMovesForBishop(result, board_, field_);
  removeKingUnveils(result, board_, this);
  return result;
}

std::vector<Figure::Move> Rook::calculatePossibleMoves() const {
  std::vector<Move> result;
  calculateMovesForRook(result, board_, field_);
  removeKingUnveils(result, board_, this);
  return result;
}

std::vector<Figure::Move> Queen::calculatePossibleMoves() const {
  std::vector<Move> result;
  calculateMovesForBishop(result, board_, field_);
  calculateMovesForRook(result, board_, field_);
  removeKingUnveils(result, board_, this);
  return result;
}

bool King::isChecked() const {
  const auto& figures = board_.getFigures();
  for (auto& figure : figures) {
    if (figure->getColor() != getColor()) {
      figure->lookForKingUnveils(false);
      auto moves = figure->calculatePossibleMoves();
      figure->lookForKingUnveils(true);
      auto iter = std::find_if(moves.begin(), moves.end(),
          [this](const auto& move) -> bool {
            return move.second == this;
          });
      if (iter != moves.end()) {
        return true;
      }
    }
  }
  return false;
}

bool King::isCheckmated() const {
  return isChecked() && calculatePossibleMoves().empty();
}

bool King::isStalemated() const {
  return isChecked() == false && calculatePossibleMoves().empty();
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
  for (auto possible_move : possible_moves) {
    bool move_is_valid = true;
    if (looksForKingUnveils()) {
      Field new_field(static_cast<Field::Letter>(possible_move.first),
                      static_cast<Field::Number>(possible_move.second));
      Board copy_board = board_;
      const Figure* figure = copy_board.getFigure(getPosition());
      assert(figure->getType() == Figure::KING);
      const King* king = static_cast<const King*>(figure);
      copy_board.moveFigure(getPosition(), new_field);
      move_is_valid = !king->isChecked();
    }
    if (move_is_valid) {
      const Figure* figure = board_.getFigure(Field(static_cast<Field::Letter>(possible_move.first),
                                                    static_cast<Field::Number>(possible_move.second)));
      addMove(result, possible_move.first, possible_move.second, figure);
    }
  }
  if (looksForKingUnveils()) {
    addPossibleCastlings(result);
  }

  return result;
}

void King::addPossibleCastlings(std::vector<Move>& moves) const {
  if (movedAtLeastOnce()) {
    return;
  }
  // Check if king is in the right position
  if ((getColor() == Figure::WHITE && getPosition() != Field(Field::E, Field::ONE)) ||
      (getColor() == Figure::BLACK && getPosition() != Field(Field::E, Field::EIGHT))) {
    return;
  }

  if (isChecked()) {
    return;
  }

  const Field::Number number = getColor() == Figure::WHITE ? Field::ONE : Field::EIGHT;

  // Check for king castling
  const Figure* figure = board_.getFigure(Field(Field::H, number));
  if (figure && figure->getType() == Figure::ROOK && figure->getColor() == getColor() &&
      figure->movedAtLeastOnce() == false && board_.getFigure(Field(Field::F, number)) == nullptr &&
      board_.getFigure(Field(Field::G, number)) == nullptr) {
    Board copy = board_;
    const Figure* f = copy.getFigure(Field(Field::E, number));
    assert(f->getType() == Figure::KING);
    const King* king = static_cast<const King*>(f);
    copy.moveFigure(Field(Field::E, number), Field(Field::F, number));
    if (king->isChecked() == false) {
      copy.moveFigure(Field(Field::F, number), Field(Field::G, number));
      if (king->isChecked() == false) {
        addMove(moves, Field::G, number, nullptr);
      }
    }
  }

  // Check for queen castling
  figure = board_.getFigure(Field(Field::A, number));
  if (figure && figure->getType() == Figure::ROOK && figure->getColor() == getColor() &&
      figure->movedAtLeastOnce() == false && board_.getFigure(Field(Field::D, number)) == nullptr &&
      board_.getFigure(Field(Field::C, number)) == nullptr &&
      board_.getFigure(Field(Field::B, number)) == nullptr) {
    Board copy = board_;
    const Figure* f = copy.getFigure(Field(Field::E, number));
    assert(f->getType() == Figure::KING);
    const King* king = static_cast<const King*>(f);
    copy.moveFigure(Field(Field::E, number), Field(Field::D, number));
    if (king->isChecked() == false) {
      copy.moveFigure(Field(Field::D, number), Field(Field::C, number));
      if (king->isChecked() == false) {
        addMove(moves, Field::C, number, nullptr);
      }
    }
  }
}

void King::move(Field field) {
  Field old_field = getPosition();
  Figure::move(field);
  const Field::Number number = getColor() == Figure::WHITE ? Field::ONE : Field::EIGHT;
  if (old_field == Field(Field::E, number)) {
    if (field == Field(Field::G, number)) {
      const Figure* rook = board_.getFigure(Field(Field::H, number));
      assert(rook && rook->getType() == Figure::ROOK);
      board_.moveFigure(Field(Field::H, number), Field(Field::F, number), false);
    } else if (field == Field(Field::C, number)) {
      const Figure* rook = board_.getFigure(Field(Field::A, number));
      assert(rook && rook->getType() == Figure::ROOK);
      board_.moveFigure(Field(Field::A, number), Field(Field::D, number), false);
    }
  }
}
