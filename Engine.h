#ifndef ENGINE_H

#include <exception>
#include <iostream>
#include <utility>
#include <vector>

#include "Board.h"
#include "Figure.h"


class Engine {
 public:
  Engine(Board& board, unsigned search_depth, std::ostream& debug_stream);
  Figure::Move makeMove(Figure::Color color);

 private:
  static const int SearchDepth = 5;

    struct Move {
      Move() {}
      Move(int v, int m, bool d) : value(v), moves_to_mate(m), is_draw(d) {}
      int value{0};
      int moves_to_mate{0};
      bool is_draw{false};
  };

  Move evaluateBoardForLastNode(const Board& board, Figure::Color color, bool my_move) const;
  Move evaluateBoard(Board& board, Figure::Color color, bool my_move, int depths_remaining) const;

  int generateRandomValue(int max) const;

  Board& board_;
  unsigned search_depth_{1u};
  std::ostream& debug_stream_;
  int moves_count_{0};
};

#define ENGINE_H
#endif  // ENGINE_H
