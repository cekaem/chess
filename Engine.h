#ifndef ENGINE_H

#include <exception>
#include <iostream>

#include "Figure.h"

class Board;


class Engine {
 public:
  enum class Status {
    NONE,
    WHITE_WON,
    BLACK_WON,
    DRAW
  };

  class GameFinishedException : public std::exception {
   public:
    GameFinishedException(Status status) : status_(status) {}
    const Status status_;
  };

  Engine(Board& board, std::ostream& debug_stream);
  Status getStatus() const;
  Status makeMove(Figure::Color color);

 private:
  int generateRandomValue(int max) const;
  Status isCheckMate() const;
  bool isStaleMate() const;
  bool isDraw() const;

  Board& board_;
  std::ostream& debug_stream_;
  int moves_count_{0};
};

inline std::ostream& operator<<(std::ostream& ostr, Engine::Status status) {
  ostr << static_cast<int>(status);
  return ostr;
}

#define ENGINE_H
#endif  // ENGINE_H
