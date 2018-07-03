#ifndef ENGINE_H

#include <exception>
#include <iostream>
#include <utility>
#include <vector>

#include "Board.h"
#include "Figure.h"


class Engine {
 public:
  Engine(Board& board, std::ostream& debug_stream);
  Figure::Move makeMove(Figure::Color color);

 private:
  static const int SearchDepth = 3;

    struct Move {
    int value{0};
    int moves_to_mate{0};
    std::vector<Move> moves;
  };

  std::vector<Move> generateTree(Board& board, Figure::Color color, int depths_remaining);
  std::pair<int, int> evaluateMoves(const Board& board, Figure::Color my_color) const;
  std::pair<int, int> evaluateMoves(const std::vector<Move>& moves,
                                    Figure::Color my_color,
                                    bool my_move) const;

  int generateRandomValue(int max) const;

  Board& board_;
  std::ostream& debug_stream_;
  int moves_count_{0};
  std::vector<Move> moves_;
};

#define ENGINE_H
#endif  // ENGINE_H
