#ifndef ENGINE_H

#include <condition_variable>
#include <exception>
#include <iostream>
#include <mutex>
#include <utility>
#include <vector>

#include "Board.h"
#include "Figure.h"
#include "Log.h"


class Engine {
 public:
  Engine(Board& board, unsigned search_depth, unsigned max_number_of_threads, std::ostream& debug_stream);
  Figure::Move makeMove(Figure::Color color);

 private:
  static const int BorderValue = 1000;

  struct Move {
    Move() {}
    Move(int v, int m, bool d) : value(v), moves_to_mate(m), is_draw(d) {}
    int value{0};
    int moves_to_mate{0};
    bool is_draw{false};
  };

  struct BorderValues {
    int the_smallest_value{BorderValue};
    int the_biggest_value{-BorderValue};
    int the_smallest_mate_value{BorderValue};
    int the_biggest_mate_value{-BorderValue};
    int the_smallest_positive_mate_value{BorderValue};
    int the_biggest_negative_mate_value{-BorderValue};
    bool zero_mate_value_exists{false};
  };

  BorderValues findBorderValues(const std::vector<Move>& moves) const;

  void evaluateBoardMain(Figure::Move move);
  Move evaluateBoardForLastNode(Board& board, Figure::Color color, bool my_move, std::vector<Figure::Move>& moves) const;
  Move evaluateBoard(Board& board, Figure::Color color, bool my_move, int depths_remaining, std::vector<Figure::Move>& moves) const;

  int generateRandomValue(int max) const;

  Board& board_;
  std::vector<std::pair<Figure::Move, Move>> evaluated_moves_;
  std::mutex evaluated_moves_mutex_;
  unsigned search_depth_{1u};
  unsigned max_number_of_threads_{1u};
  unsigned number_of_threads_working_{0u};
  std::mutex number_of_threads_working_mutex_;
  std::condition_variable number_of_threads_working_cv_;
  mutable logging::Log debug_stream_;
  int moves_count_{0};
};

#define ENGINE_H
#endif  // ENGINE_H
