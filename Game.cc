#include <iostream>
#include <string>
#include <utility>

#include "Board.h"
#include "Engine.h"
#include "Figure.h"
#include "PgnCreator.h"

namespace {

class TextBoardDrawer : public BoardDrawer {
 public:
  TextBoardDrawer(std::ostream& ostr) : ostr_(ostr) {}

  void onFigureAdded(Figure::Type type, Figure::Color color, Field field) override {
  }

  void onFigureRemoved(Field field) override {
  }

  void onGameFinished(Board::GameStatus status) override {
    switch (status) {
      case Board::GameStatus::NONE:
        break;
      case Board::GameStatus::WHITE_WON:
        ostr_ << "1-0";
        break;
      case Board::GameStatus::BLACK_WON:
        ostr_ << "0-1";
        break;
      case Board::GameStatus::DRAW:
        ostr_ << "1/2-1/2";
        break;
    }
    ostr_ << std::endl;
  }

  void onFigureMoved(Figure::Move move) override {
    ostr_ << move.old_field;
    if (move.figure_beaten) {
     ostr_ << "x";
    } else {
     ostr_ << "-";
    }
    ostr_ << move.new_field;
    switch (move.pawn_promotion) {
      case Figure::PAWN:
      case Figure::KING:
        break;
      case Figure::BISHOP:
        ostr_ << "B";
        break;
      case Figure::KNIGHT:
        ostr_ << "K";
        break;
      case Figure::ROOK:
        ostr_ << "R";
        break;
      case Figure::QUEEN:
        ostr_ << "Q";
        break;
    }
    if (move.is_mate) {
      ostr_ << "#";
    }else if (move.is_check) {
      ostr_ << "+";
    }
    ostr_ << std::endl;
  }

 private:
  std::ostream& ostr_;
};

std::tuple<Field, Field, Figure::Type> getHumanMove(Board& board) {
  bool move_ok = false;
  Field old_field;
  Field new_field;
  while (move_ok == false) {
    bool field_ok = false;
    while (field_ok == false) {
      std::cout << "> ";
      std::string field;
      std::cin >> field;
      if (Field::isFieldValid(field) == true) {
        field_ok = true;
        old_field = Field(field.c_str());
      } else {
        std::cerr << "Invalid field" << std::endl;
      }
    }
    field_ok = false;
    while (field_ok == false) {
      std::cout << ">> ";
      std::string field;
      std::cin >> field;
      if (Field::isFieldValid(field) == true) {
        field_ok = true;
        new_field = Field(field.c_str());
      } else {
        std::cerr << "Invalid field" << std::endl;
      }
    }
    if (board.isMoveValid(old_field, new_field) == true) {
      move_ok = true;
    } else {
      std::cerr << "Invalid move." << std::endl;
    }
  }
  Figure::Type promotion_to = Figure::PAWN;
  if (Figure::Move::isPromotion(&board, old_field, new_field) == true) {
    bool promotion_ok = false;
    while (promotion_ok == false) {
      std::cout << "Promotion to?" << std::endl;
      std::cout << ">>> ";
      std::string promotion_string;
      std::cin >> promotion_string;
      if (promotion_string.size() != 1) {
        continue;
      }
      switch (promotion_string[0]) {
        case 'Q':
        case 'q':
          promotion_to = Figure::QUEEN;
          promotion_ok = true;
          break;
        case 'R':
        case 'r':
          promotion_to = Figure::ROOK;
          promotion_ok = true;
          break;
        case 'N':
        case 'n':
          promotion_to = Figure::KNIGHT;
          promotion_ok = true;
          break;
        case 'B':
        case 'b':
          promotion_to = Figure::BISHOP;
          promotion_ok = true;
          break;
        default:
          break;
      }
    }
  }
  return std::make_tuple(old_field, new_field, promotion_to);
}

}  // unnamed namespace

int main() {
  bool human_plays_white = false;
  bool human_plays_black = false;
  int engine_strength = 0;
  int number_of_threads = 0;

  std::cout << "Human plays white?" << std::endl;
  std::cout << "> ";
  std::cin >> human_plays_white;
  std::cout << "Human plays black?" << std::endl;
  std::cout << "> ";
  std::cin >> human_plays_black;
  std::cout << "Engine's search depth?" << std::endl;
  std::cout << "> ";
  std::cin >> engine_strength;
  std::cout << "Number of threads?" << std::endl;
  std::cout << "> ";
  std::cin >> number_of_threads;

  Board board;
  board.setStandardBoard();
  TextBoardDrawer drawer(std::cout);
  PgnCreator pgn_creator(std::cout);
  board.addBoardDrawer(&drawer);
  board.addBoardDrawer(&pgn_creator);
  Engine engine(board, engine_strength, number_of_threads, std::cerr);
  
  Board::GameStatus status = Board::GameStatus::NONE;
  while (status == Board::GameStatus::NONE) {
    if (human_plays_white == true) {
      auto human_move = getHumanMove(board);
      status = board.makeMove(std::get<0>(human_move),
                              std::get<1>(human_move),
                              std::get<2>(human_move));
    } else {
      engine.makeMove();
      status = board.getGameStatus(Figure::BLACK);
    }
    if (status == Board::GameStatus::NONE) {
      if (human_plays_black == true) {
        auto human_move = getHumanMove(board);
        status = board.makeMove(std::get<0>(human_move),
                                std::get<1>(human_move),
                                std::get<2>(human_move));
      } else {
        engine.makeMove();
        status = board.getGameStatus(Figure::WHITE);
      }
    }
  }

  return 0;
}
