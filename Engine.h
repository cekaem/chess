#ifndef ENGINE_H
#define ENGINE_H

#include <condition_variable>
#include <exception>
#include <iostream>
#include <mutex>
#include <utility>
#include <vector>

#include "Board.h"
#include "Figure.h"
#include "utils/SocketLog.h"


class Engine {
 public:
  Engine(Board& board, unsigned search_depth, unsigned max_number_of_threads, utils::SocketLog& debug_stream);
  Engine(Board& board, utils::SocketLog& debug_stream);
  ~Engine();
  void setNumberOfThreads(unsigned number_of_threads) { max_number_of_threads_ = number_of_threads; }
  void setSearchDepth(unsigned search_depth) { search_depth_ = search_depth_; }
  Figure::Move makeMove();

 private:
  static const int BorderValue = 1000;
  static const unsigned DefaultSearchDepth = 3;
  static const unsigned DefaultNumberOfThreads = 5;

  struct Move {
    Move() {}
    Move(Figure::Move fmove, int v, int m, bool d) : move(fmove), value(v), moves_to_mate(m), is_draw(d) {}
    Figure::Move move;
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
  Move evaluateBoardForLastNode(Board& board,
                                const Figure::Move& move,
                                Figure::Color color,
                                bool my_move,
                                std::vector<Figure::Move>& moves) const;
  Move evaluateBoard(Board& board,
                     const Figure::Move& move,
                     Figure::Color color,
                     bool my_move,
                     int depths_remaining,
                     std::vector<Figure::Move>& moves) const;

  int generateRandomValue(int max) const;

  Board& board_;
  utils::SocketLog& debug_stream_;
  std::vector<std::pair<Figure::Move, Move>> evaluated_moves_;
  std::mutex evaluated_moves_mutex_;
  unsigned search_depth_{DefaultSearchDepth};
  unsigned max_number_of_threads_{DefaultNumberOfThreads};
  unsigned number_of_threads_working_{0u};
  std::mutex number_of_threads_working_mutex_;
  std::condition_variable number_of_threads_working_cv_;
  int moves_count_{0};
};

#endif  // ENGINE_H
