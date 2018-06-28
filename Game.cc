#include <iostream>

#include "Board.h"
#include "Engine.h"
#include "Figure.h"

namespace {

class TextBoardDrawer : public BoardDrawer {
 public:
  TextBoardDrawer(std::ostream& ostr) : ostr_(ostr) {}

  void onFigureAdded(Figure::Type type, Figure::Color color, Field field) override {
  }

  void onFigureRemoved(Field field) override {
  }

  void onFigureMoved(Field start_field, Field end_field, bool beaten_figure, bool is_check) override {
    ostr_ << start_field;
    if (beaten_figure) {
     ostr_ << "x";
    } else {
     ostr_ << "-";
    }
    ostr_ << end_field;
    if (is_check) {
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
  board.addBoardDrawer(&drawer);
  Engine engine(board, std::cerr);
  Figure::Color color = Figure::WHITE;
  while(engine.makeMove(color)) {
    color = !color;
  }
  std::cout << board << std::endl;

  return 0;
}