#include "PgnCreator.h"

#include "Board.h"
#include "Figure.h"

void PgnCreator::onGameFinished(Board::GameStatus status) {
  switch (status) {
    case Board::GameStatus::NONE:
      break;
    case Board::GameStatus::WHITE_WON:
      ss_ << "1-0";
      ostr_ << "[Result \"1-0\"]" << std::endl;
      break;
    case Board::GameStatus::BLACK_WON:
      ss_ << "0-1";
      ostr_ << "[Result \"0-1\"]" << std::endl;
      break;
    case Board::GameStatus::DRAW:
      ss_ << "1/2-1/2";
      ostr_ << "[Result \"1/2-1/2\"]" << std::endl;
      break;
  }

  ostr_ << ss_.str() << std::endl;
}

void PgnCreator::onFigureMoved(Figure::Move move) {
  ss_ << move.old_field;
  if (move.figure_beaten) {
   ss_ << "x";
  } else {
   ss_ << "-";
  }
  ss_ << move.new_field;
  switch (move.pawn_promotion) {
    case Figure::PAWN:
    case Figure::KING:
      break;
    case Figure::BISHOP:
      ss_ << "B";
      break;
    case Figure::KNIGHT:
      ss_ << "N";
      break;
    case Figure::ROOK:
      ss_ << "R";
      break;
    case Figure::QUEEN:
      ss_ << "Q";
      break;
  }
  if (move.is_mate) {
    ss_ << "#";
  } else if (move.is_check) {
    ss_ << "+";
  }
  ss_ << std::endl;
}
