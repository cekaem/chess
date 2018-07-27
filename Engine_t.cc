/* Component tests for class Engine */

#include <exception>
#include <iostream>
#include <stdexcept>
#include <utility>

#include "utils/SocketLog.h"
#include "utils/Test.h"
#include "Board.h"
#include "Engine.h"
#include "Field.h"
#include "Figure.h"

namespace {

bool MovesEqual(Figure::Move move1,
                const std::string& move2,
                Figure::Type promotion = Figure::PAWN) {
  if (move2.size() != 5 || move2[2] != '-') {
    throw std::runtime_error("MovesEqual: wrong move");
  }
  return move1.old_field == Field(move2.substr(0, 2).c_str()) &&
         move1.new_field == Field(move2.substr(3, 2).c_str()) &&
         move1.pawn_promotion == promotion;
}

// Checks if engine detects mate in one
TEST_PROCEDURE(test1) {
  TEST_START
  {
    Board board;
    Engine engine(board, 3, 4, Engine::LogSection::NONE);
    board.addFigure(Figure::KING, Field("a8"), Figure::BLACK);
    board.addFigure(Figure::KING, Field("c7"), Figure::WHITE);
    board.addFigure(Figure::ROOK, Field("f6"), Figure::WHITE);
    auto move = engine.makeMove();
    VERIFY_TRUE(MovesEqual(move, "f6-a6"));
  }
  {
    Board board;
    Engine engine(board, 3, 4, Engine::LogSection::NONE);
    board.addFigure(Figure::KING, Field("a7"), Figure::BLACK);
    board.addFigure(Figure::KING, Field("h1"), Figure::WHITE);
    board.addFigure(Figure::BISHOP, Field("d5"), Figure::WHITE);
    board.addFigure(Figure::BISHOP, Field("f4"), Figure::WHITE);
    board.addFigure(Figure::PAWN, Field("c7"), Figure::WHITE);
    board.addFigure(Figure::PAWN, Field("a6"), Figure::BLACK);
    board.addFigure(Figure::PAWN, Field("b6"), Figure::BLACK);
    auto move = engine.makeMove();
    VERIFY_TRUE(MovesEqual(move, "c7-c8", Figure::KNIGHT));
  }
  {
    Board board;
    Engine engine(board, 3, 4, Engine::LogSection::NONE);
    board.setBoardFromFEN("8/1b6/8/8/7p/4p1P1/6nP/4k1BK b - - 0 1");
    auto move = engine.makeMove();
    VERIFY_TRUE(MovesEqual(move, "g2-f4"));
  }
  {
    Board board;
    Engine engine(board, 1, 1, Engine::LogSection::NONE);
    board.addFigure(Figure::KING, Field("b8"), Figure::BLACK);
    board.addFigure(Figure::KING, Field("d8"), Figure::WHITE);
    board.addFigure(Figure::PAWN, Field("b5"), Figure::WHITE);
    board.addFigure(Figure::ROOK, Field("b1"), Figure::WHITE);
    board.addFigure(Figure::BISHOP, Field("e3"), Figure::WHITE);
    board.addFigure(Figure::BISHOP, Field("e4"), Figure::WHITE);
    board.addFigure(Figure::PAWN, Field("a7"), Figure::BLACK);
    board.setSideToMove(Figure::BLACK);
    board.makeMove(Field("a7"), Field("a5"));
    auto move = engine.makeMove();
    VERIFY_TRUE(MovesEqual(move, "b5-a6"));
  }
 TEST_END
}

// Checks if engine grabs free material
TEST_PROCEDURE(test2) {
  TEST_START
  {
    Board board;
    Engine engine(board, 3, 4, Engine::LogSection::NONE);
    board.addFigure(Figure::KING, Field("a8"), Figure::BLACK);
    board.addFigure(Figure::KING, Field("d2"), Figure::WHITE);
    board.addFigure(Figure::QUEEN, Field("d1"), Figure::BLACK);
    auto move = engine.makeMove();
    VERIFY_TRUE(MovesEqual(move, "d2-d1"));
  }
  TEST_END
}

// Checks if engine promotes to Queen when it has chance
TEST_PROCEDURE(test3) {
  TEST_START
  Board board;
  Engine engine(board, 3, 4, Engine::LogSection::NONE);
  board.addFigure(Figure::KING, Field("a6"), Figure::WHITE);
  board.addFigure(Figure::KING, Field("d4"), Figure::BLACK);
  board.addFigure(Figure::PAWN, Field("h7"), Figure::WHITE);
  auto move = engine.makeMove();
  VERIFY_TRUE(MovesEqual(move, "h7-h8", Figure::QUEEN));
  TEST_END
}

// Checks if engine detects mate in two.
TEST_PROCEDURE(test4) {
  TEST_START
  {
    Board board;
    Engine engine(board, 3, 4, Engine::LogSection::NONE);
    board.setBoardFromFEN("8/8/1b6/1k6/3q4/3n4/6PP/R3R2K b - - 0 1");
    auto move = engine.makeMove();
    VERIFY_TRUE(MovesEqual(move, "d4-g1"));
    move = engine.makeMove();
    VERIFY_TRUE(MovesEqual(move, "e1-g1"));
    move = engine.makeMove();
    VERIFY_TRUE(MovesEqual(move, "d3-f2"));
  }  
  TEST_END
}

// Checks if Engine detects proper move to avoid mate in two.
TEST_PROCEDURE(test5) {
  TEST_START
  {
    Board board;
    Engine engine(board, 4, 4, Engine::LogSection::NONE);
    board.setBoardFromFEN("6k1/5ppp/6b1/3Q3n/1K6/8/8/8 b - - 0 1");
    auto move = engine.makeMove();
    VERIFY_TRUE(MovesEqual(move, "h7-h6"));
  }
  TEST_END
}

} // unnamed namespace


int main() {
  try {
    TEST("Engine detects mate in one", test1);
    TEST("Engine grabs free material", test2);
    TEST("Engine promotes pawns when it has chance", test3);
    TEST("Engine detects mate in two", test4);
    TEST("Engine detects proper move to avoid mate in two", test5);
  } catch (std::exception& except) {
    std::cerr << "Unexpected exception: " << except.what() << std::endl;
     return -1;
  }
  std::cout << "Number of copies: " << Board::number_of_copies_ << std::endl;
  int failed_tests = Test::get_number_of_failed_tests();
  if (failed_tests > 0) {
    std::cout << failed_tests << " test(s) failed." << std::endl;
    return -2;
  }
  std::cout << "All tests passed." << std::endl;
  return 0;
}
