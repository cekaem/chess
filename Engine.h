#ifndef ENGINE_H
#define ENGINE_H

#include <condition_variable>
#include <exception>
#include <iostream>
#include <mutex>
#include <vector>

#include "Board.h"
#include "Figure.h"
#include "utils/Timer.h"


class Engine {
 public:
  Engine(Board& board, unsigned search_depth, unsigned max_number_of_threads);
  Engine(Board& board);
  void setNumberOfThreads(unsigned number_of_threads) { max_number_of_threads_ = number_of_threads; }
  void setSearchDepth(unsigned search_depth) { search_depth_ = search_depth_; }
  Figure::Move makeMove(unsigned time_for_move = 0u);

 private:
  static const int BorderValue = 1000;
  static const unsigned DefaultSearchDepth = 3;
  static const unsigned DefaultNumberOfThreads = 5;

  struct Move {
    Move() {}
    Move(Figure::Move fmove, Engine::Move* p) : move(fmove), parent(p) {}
    Move(Figure::Move fmove, int v, int m, bool d) : move(fmove), value(v), moves_to_mate(m), is_draw(d) {}
    Figure::Move move;
    int value{0};
    int moves_to_mate{0};
    bool is_draw{false};
    std::vector<Engine::Move> moves;
    Engine::Move* parent{nullptr};
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

  void evaluateBoardForLastNode(Board& board, Move& move) const;
  void evaluateBoard(Board& board, Move& move) const;

  void generateTreeMain(Engine::Move& move);
  void generateTree(Board& board, Figure::Color color, Engine::Move& move);
  
  void onThreadFinished();

  int generateRandomValue(int max) const;

  void onTimerExpired();

  Board& board_;
  unsigned search_depth_{DefaultSearchDepth};
  unsigned max_number_of_threads_{DefaultNumberOfThreads};
  unsigned number_of_threads_working_{0u};
  std::mutex number_of_threads_working_mutex_;
  std::condition_variable number_of_threads_working_cv_;
  int moves_count_{0};
  utils::Timer timer_;
  bool timer_expired_{false};
};

#endif  // ENGINE_H
