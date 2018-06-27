#ifndef ENGINE_H

#include <iostream>

#include "Figure.h"

class Board;


class Engine {
 public:
  Engine(Board& board, std::ostream& debug_stream);
  bool isCheckMate() const;
  bool isStaleMate() const;
  bool isDraw() const;
  bool makeMove(Figure::Color color);

 private:
  int generateRandomValue(int max) const;

  Board& board_;
  std::ostream& debug_stream_;
  int moves_count_{0};
};

#define ENGINE_H
#endif  // ENGINE_H
