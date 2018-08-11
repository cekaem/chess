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
#include "utils/Timer.h"


class Engine {
 public:
  struct SearchInfo {
    unsigned depth{0u};
    unsigned nodes{0u};
    int score_cp{0};
    int score_mate{0};
    unsigned time{0u};
    std::vector<Figure::Move> best_line;
  };

  Engine(Board& board, unsigned max_number_of_threads);
  Engine(Board& board);
  void setNumberOfThreads(unsigned number_of_threads) { max_number_of_threads_ = number_of_threads; }
  void setMaxMemoryConsumption(unsigned m) { max_memory_consumption_ = m; }
  SearchInfo startSearch(unsigned time_for_move, unsigned search_depth);
  Figure::Move makeMove(unsigned time_for_move = 0u, unsigned search_depth = DefaultSearchDepth);
  void endCalculations() { end_calculations_ = true; }

 private:
  static const int BorderValue = 100000;
  static const unsigned DefaultSearchDepth = 4;
  static const unsigned DefaultNumberOfThreads = 5;
  static const unsigned DefaultMaxMemoryConsumption = 2000000u;  // kB. ~2GB

  struct Move {
    Move() {}
    Move(const Move& other) {
      // Created to avoid unnecessary coping of field moves
      move = other.move;
      value_cp = other.value_cp;
      moves_to_mate = other.moves_to_mate;
      is_draw = other.is_draw;
      parent = other.parent;
    }
    Move(Figure::Move fmove, Engine::Move* p) : move(fmove), parent(p) {}
    Move(Figure::Move fmove, int v, int m, bool d) : move(fmove), value_cp(v), moves_to_mate(m), is_draw(d) {}

    Figure::Move move;
    int value_cp{0};
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
  std::pair<int, int> evaluateBorderValues(BorderValues values, Figure::Color color) const;
  int calculateMoveModificator(Board& board, const Move& move) const;

  void generateTreeMain(Engine::Move& move);
  void generateTree(Board& board, Figure::Color color, Engine::Move& move);

  Move* lookForTheBestMove(std::vector<Engine::Move>& moves, Figure::Color color) const;
  
  void onThreadFinished();

  int generateRandomValue(int max) const;

  void onTimerExpired();

  void onMaxMemoryConsumptionExceeded(unsigned memory_consumption);

  Board& board_;
  unsigned max_number_of_threads_{DefaultNumberOfThreads};
  unsigned max_memory_consumption_{DefaultMaxMemoryConsumption};
  unsigned number_of_threads_working_{0u};
  std::mutex number_of_threads_working_mutex_;
  std::condition_variable number_of_threads_working_cv_;
  int moves_count_{0};
  unsigned nodes_evaluated_{0u};
  utils::Timer timer_;
  bool end_calculations_{false};
};

#endif  // ENGINE_H
