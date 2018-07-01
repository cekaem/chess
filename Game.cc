#include <iostream>

#include "Board.h"
#include "Engine.h"
#include "Figure.h"
#include "PgnCreator.h"

namespace {

class TextBoardDrawer : public BoardDrawer {
 public:
  TextBoardDrawer(std::ostream& ostr) : ostr_(ostr) {}

  void onFigureAdded(Figure::Type type, Figure::Color color, Field field) override {
  }

  void onFigureRemoved(Field field) override {
  }

  void onGameFinished(Board::GameStatus status) override {
    switch (status) {
      case Board::GameStatus::NONE:
        break;
      case Board::GameStatus::WHITE_WON:
        ostr_ << "1-0";
        break;
      case Board::GameStatus::BLACK_WON:
        ostr_ << "0-1";
        break;
      case Board::GameStatus::DRAW:
        ostr_ << "1/2-1/2";
        break;
    }
    ostr_ << std::endl;
  }

  void onFigureMoved(Figure::Move move) override {
    ostr_ << move.old_field;
    if (move.figure_beaten) {
     ostr_ << "x";
    } else {
     ostr_ << "-";
    }
    ostr_ << move.new_field;
    switch (move.pawn_promotion) {
      case Figure::PAWN:
      case Figure::KING:
        break;
      case Figure::BISHOP:
        ostr_ << "B";
        break;
      case Figure::KNIGHT:
        ostr_ << "K";
        break;
      case Figure::ROOK:
        ostr_ << "R";
        break;
      case Figure::QUEEN:
        ostr_ << "Q";
        break;
    }
    if (move.is_mate) {
      ostr_ << "#";
    }else if (move.is_check) {
      ostr_ << "+";
    }
    ostr_ << std::endl;
  }

 private:
  std::ostream& ostr_;
};

}  // unnamed namespace

int main() {
  Board board;
  board.setStandardBoard();
  TextBoardDrawer drawer(std::cout);
  PgnCreator pgn_creator(std::cout);
  board.addBoardDrawer(&drawer);
  board.addBoardDrawer(&pgn_creator);
  Engine engine(board, std::cerr);
  Figure::Color color = Figure::WHITE;
  Board::GameStatus status = Board::GameStatus::NONE;
  while((status = engine.makeMove(color)) == Board::GameStatus::NONE) {
    color = !color;
  }
  std::cout << board << std::endl;

  return 0;
}
