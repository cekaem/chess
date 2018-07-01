#ifndef PGN_CREATOR_H
#define PGN_CREATOR_H

#include <ostream>
#include <sstream>

#include "Board.h"

class PgnCreator : public BoardDrawer {
 public:
  PgnCreator(std::ostream& ostr) : ostr_(ostr) {}

  void onFigureAdded(Figure::Type type, Figure::Color color, Field field) override {}
  void onFigureRemoved(Field field) override {}
  void onFigureMoved(Figure::Move move) override;
  void onGameFinished(Board::GameStatus status) override;

 private:
  std::ostream& ostr_;
  std::stringstream ss_;
};

#endif  // PGN_CREATOR_H
