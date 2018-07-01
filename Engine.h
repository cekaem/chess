#ifndef ENGINE_H

#include <exception>
#include <iostream>
#include <utility>

#include "Board.h"
#include "Figure.h"


class Engine {
 public:
  Engine(Board& board, std::ostream& debug_stream);
  Figure::Move makeMove(Figure::Color color);

 private:
  int generateRandomValue(int max) const;

  Board& board_;
  std::ostream& debug_stream_;
  int moves_count_{0};
};

#define ENGINE_H
#endif  // ENGINE_H
