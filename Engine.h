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
  static const int BorderValue = 100;

  struct Move {
    Move(Board& b) : board(b) {}
    Board board;
    Figure::Move basic_data;
    int value{0};
    int moves_to_mate{0};
    bool moves_calculated{false};
    std::vector<Move> moves;

    void calculateMoves();
  };

  std::pair<int, int> evaluatePosition(const Board& board, Figure::Color my_color) const;
  std::pair<int, int> evaluatePosition(const std::vector<Move>& moves,
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
