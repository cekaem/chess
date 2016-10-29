#include "Figure.h"

#include <algorithm>
#include <cassert>

namespace {

void addMove(std::vector<Figure::Move>& moves, unsigned l, unsigned n, const Figure* f) {
  auto move = std::make_pair(std::make_pair(static_cast<Board::Letter>(l),
                                       static_cast<Board::Number>(n)),
                        f);
  moves.push_back(move);
}

void calculateMovesForBishop(std::vector<Figure::Move>& moves, const Board& board, const Board::Field field) {
  Figure::Color my_color = board.getFigure(field)->getColor();

  int l = field.first - 1;
  int n = field.second - 1;
  while (l >= Board::A && n >= Board::ONE) {
    const Figure* figure = board.getFigure(std::make_pair(static_cast<Board::Letter>(l),
                                                          static_cast<Board::Number>(n)));
    if (!figure || (figure && figure->getColor() != my_color)) {
      addMove(moves, l, n, figure);
    }
    if (figure != nullptr) {
      break;
    }
    --l, --n;
  }

  l = field.first - 1;
  n = field.second + 1;
  while (l >= Board::A && n <= Board::EIGHT) {
    const Figure* figure = board.getFigure(std::make_pair(static_cast<Board::Letter>(l),
                                                          static_cast<Board::Number>(n)));
    if (!figure || (figure && figure->getColor() != my_color)) {
      addMove(moves, l, n, figure);
    }
    if (figure != nullptr) {
      break;
    }
    --l, ++n;
  }

  l = field.first + 1;
  n = field.second + 1;
  while (l <= Board::H && n <= Board::EIGHT) {
    const Figure* figure = board.getFigure(std::make_pair(static_cast<Board::Letter>(l),
                                                          static_cast<Board::Number>(n)));
    if (!figure || (figure && figure->getColor() != my_color)) {
      addMove(moves, l, n, figure);
    }
    if (figure != nullptr) {
      break;
    }
    ++l, ++n;
  }

  l = field.first + 1;
  n = field.second - 1;
  while (l <= Board::H && n >= Board::ONE) {
    const Figure* figure = board.getFigure(std::make_pair(static_cast<Board::Letter>(l),
                                                          static_cast<Board::Number>(n)));
    if (!figure || (figure && figure->getColor() != my_color)) {
      addMove(moves, l, n, figure);
    }
    if (figure != nullptr) {
      break;
    }
    ++l, --n;
  }
}

void calculateMovesForRook(std::vector<Figure::Move>& moves, const Board& board, const Board::Field field) {
  Figure::Color my_color = board.getFigure(field)->getColor();

  int l = field.first - 1;
  int n = field.second;
  while (l >= Board::A) {
    const Figure* figure = board.getFigure(std::make_pair(static_cast<Board::Letter>(l),
                                                          static_cast<Board::Number>(n)));
    if (!figure || (figure && figure->getColor() != my_color)) {
      addMove(moves, l, n, figure);
    }
    if (figure != nullptr) {
      break;
    }
    --l;
  }

  l = field.first + 1;
  while (l <= Board::H) {
    const Figure* figure = board.getFigure(std::make_pair(static_cast<Board::Letter>(l),
                                                          static_cast<Board::Number>(n)));
    if (!figure || (figure && figure->getColor() != my_color)) {
      addMove(moves, l, n, figure);
    }
    if (figure != nullptr) {
      break;
    }
    ++l;
  }

  l = field.first;
  n = field.second + 1;
  while (n <= Board::EIGHT) {
    const Figure* figure = board.getFigure(std::make_pair(static_cast<Board::Letter>(l),
                                                          static_cast<Board::Number>(n)));
    if (!figure || (figure && figure->getColor() != my_color)) {
      addMove(moves, l, n, figure);
    }
    if (figure != nullptr) {
      break;
    }
    ++n;
  }

  n = field.second - 1;
  while (n >= Board::ONE) {
    const Figure* figure = board.getFigure(std::make_pair(static_cast<Board::Letter>(l),
                                                          static_cast<Board::Number>(n)));
    if (!figure || (figure && figure->getColor() != my_color)) {
      addMove(moves, l, n, figure);
    }
    if (figure != nullptr) {
      break;
    }
    --n;
  }
}

}  // unnamed namespace

Figure::Figure(Board& board, Board::Field field, Color color, int value)
  throw(Board::WrongFieldException, Board::FieldNotEmptyException)
  : board_(board), field_(field), color_(color), value_(value) {
  board_.addFigure(this, field);
}

void Figure::move(Board::Field field) throw(IllegalMoveException) {
  auto possible_moves = calculatePossibleMoves();
  auto iter = std::find_if(possible_moves.begin(), possible_moves.end(),
      [field](const auto& iter) -> bool {
        return field == iter.first;
      });
  if (iter == possible_moves.end()) {
    throw IllegalMoveException(this, field);
  }
  const Figure* figure = board_.removeFigure(field_);
  assert(figure == this);
  board_.addFigure(this, field);
  field_ = field;
  board_.setEnPassantPawn(nullptr);
}

void Pawn::move(Board::Field field) throw(IllegalMoveException) {
  Board::Field old_field = field_;
  Figure::move(field);
  if ((color_ == WHITE && old_field.second == Board::TWO && field.second == Board::FOUR) ||
      (color_ == BLACK && old_field.second == Board::SEVEN && field.second == Board::FIVE)) {
    board_.setEnPassantPawn(this);
  }
}

std::vector<Figure::Move> Pawn::calculatePossibleMoves() const {
  std::vector<Move> result;
  Board::Letter current_l = field_.first;
  Board::Number current_n = field_.second;

  if ((color_ == WHITE && current_n == Board::EIGHT) ||
      (color_ == BLACK && current_n == Board::ONE)) {
    return result;
  }

  const auto& fields = board_.getFields();
  const int offset = color_ == WHITE ? 1 : -1;

  if (fields[current_l][current_n + offset] == nullptr) {
    addMove(result, current_l, current_n + offset, nullptr);
  }

  if ((color_ == WHITE && field_.second == Board::TWO) ||
      (color_ == BLACK && field_.second == Board::SEVEN)) {
    if (fields[current_l][current_n + offset] == nullptr &&
        fields[current_l][current_n + 2 * offset] == nullptr) {
      addMove(result, current_l, current_n + 2 * offset, nullptr);
    }
  }

  if (current_l != Board::A) {
    const Figure* figure = fields[current_l - 1][current_n + offset];
    if (figure != nullptr && figure->getColor() != color_) {
      addMove(result, current_l - 1, current_n + offset, figure);
    }
  }

  if (current_l != Board::H) {
    const Figure* figure = fields[current_l + 1][current_n + offset];
    if (figure != nullptr && figure->getColor() != color_) {
      addMove(result, current_l + 1, current_n + offset, figure);
    }
  }

  // Check for "en passant"
  const Pawn* en_passant = board_.getEnPassantPawn();
  if (en_passant != nullptr) {
    if ((color_ == WHITE && field_.second == Board::FIVE) ||
        (color_ == BLACK && field_.second == Board::FOUR)) {
      if (field_.first != Board::A && fields[field_.first - 1][field_.second] == en_passant) {
        addMove(result, current_l - 1, current_n + offset, en_passant);
      } else if (field_.first != Board::H && fields[field_.first + 1][field_.second] == en_passant) {
        addMove(result, current_l + 1, current_n + offset, en_passant);
      }
    }
  }

  return result;
}

std::vector<Figure::Move> Knight::calculatePossibleMoves() const {
  std::vector<Move> result;
  Board::Letter current_l = field_.first;
  Board::Number current_n = field_.second;

  std::vector<std::pair<int, int>> possibleMoves;
  possibleMoves.push_back(std::make_pair(current_l + 2, current_n - 1));
  possibleMoves.push_back(std::make_pair(current_l + 2, current_n + 1));
  possibleMoves.push_back(std::make_pair(current_l + 1, current_n + 2));
  possibleMoves.push_back(std::make_pair(current_l - 1, current_n + 2));
  possibleMoves.push_back(std::make_pair(current_l - 2, current_n - 1));
  possibleMoves.push_back(std::make_pair(current_l - 2, current_n + 1));
  possibleMoves.push_back(std::make_pair(current_l + 1, current_n - 2));
  possibleMoves.push_back(std::make_pair(current_l - 1, current_n - 2));

  possibleMoves.erase(std::remove_if(possibleMoves.begin(), possibleMoves.end(),
                 [](const auto& iter) -> bool {
                   return iter.first < Board::A || iter.first > Board::H ||
                          iter.second < Board::ONE || iter.second > Board::EIGHT;
                 }), possibleMoves.end());

  for (const auto& iter : possibleMoves) {
    Board::Field field = std::make_pair(static_cast<Board::Letter>(iter.first),
                                        static_cast<Board::Number>(iter.second));
    const Figure* figure = board_.getFigure(field);
    if (figure == nullptr || (figure != nullptr  && figure->getColor() != color_)) {
      addMove(result, field.first, field.second, figure);
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
