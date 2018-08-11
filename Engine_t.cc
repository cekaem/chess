/* Component tests for class Engine */

#include <cassert>
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


TEST_PROCEDURE(EngineDetectsMateInOne) {
  TEST_START
  {
    Board board;
    Engine engine(board, 4);
    board.addFigure(Figure::KING, Field("a8"), Figure::BLACK);
    board.addFigure(Figure::KING, Field("c7"), Figure::WHITE);
    board.addFigure(Figure::ROOK, Field("f6"), Figure::WHITE);
    auto move = engine.makeMove();
    VERIFY_TRUE(MovesEqual(move, "f6-a6"));
  }
  {
    Board board;
    Engine engine(board, 4);
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
    Engine engine(board, 4);
    board.setBoardFromFEN("8/1b6/8/8/7p/4p1P1/6nP/4k1BK b - - 0 1");
    auto move = engine.makeMove();
    VERIFY_TRUE(MovesEqual(move, "g2-f4"));
  }
  {
    Board board;
    Engine engine(board, 1);
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

TEST_PROCEDURE(EngineGrabsFreeMaterial) {
  TEST_START
  {
    Board board;
    Engine engine(board, 4);
    board.addFigure(Figure::KING, Field("a8"), Figure::BLACK);
    board.addFigure(Figure::KING, Field("d2"), Figure::WHITE);
    board.addFigure(Figure::QUEEN, Field("d1"), Figure::BLACK);
    auto move = engine.makeMove();
    VERIFY_TRUE(MovesEqual(move, "d2-d1"));
  }
  TEST_END
}

TEST_PROCEDURE(EnginePromotesToQueen) {
  TEST_START
  Board board;
  Engine engine(board, 4);
  board.addFigure(Figure::KING, Field("a6"), Figure::WHITE);
  board.addFigure(Figure::KING, Field("d4"), Figure::BLACK);
  board.addFigure(Figure::PAWN, Field("h7"), Figure::WHITE);
  auto move = engine.makeMove();
  VERIFY_TRUE(MovesEqual(move, "h7-h8", Figure::QUEEN));
  TEST_END
}

TEST_PROCEDURE(EngineDetectsMateInTwo) {
  TEST_START
  {
    Board board;
    Engine engine(board, 4);
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

TEST_PROCEDURE(EngineAvoidsMateInTwo) {
  TEST_START
  {
    Board board;
    Engine engine(board, 4);
    board.setBoardFromFEN("6k1/5ppp/6b1/3Q3n/1K6/8/8/8 b - - 0 1");
    auto move = engine.makeMove();
    VERIFY_TRUE(MovesEqual(move, "h7-h6"));
  }
  TEST_END
}

TEST_PROCEDURE(EngineBorderValueIsBigEnough) {
  TEST_START
  Board board;
  Engine engine(board, 4);
  board.setBoardFromFEN("8/3k4/8/8/8/8/4K1QR/8 b - - 0 35");
  // If Engine::BorderValue is exceeded makeMove will cause assert
  engine.makeMove(100, 2);
  TEST_END
}

TEST_PROCEDURE(EngineFindsProperMoveWithSearchDepthSetToOne) {
  TEST_START
  Board board;
  Engine engine(board, 4);
  board.setBoardFromFEN("8/8/4k3/4Q3/8/8/5K2/8 b - - 0 37");
  auto move = engine.makeMove(500, 1);
  VERIFY_TRUE(MovesEqual(move, "e6-e5"));
  TEST_END
}

TEST_PROCEDURE(EngineCastlesWhenItIsDesirable) {
  TEST_START
  Board board;
  Engine engine(board, 4);
  board.setBoardFromFEN("4k2r/8/8/8/8/8/8/3K4 b - - 0 1");
  auto move = engine.makeMove(1000, 2);
  VERIFY_FALSE(MovesEqual(move, "e8-g8"));
  board.setBoardFromFEN("4k2r/8/8/8/8/8/8/3K4 b k - 0 1");
  move = engine.makeMove(1000, 2);
  VERIFY_EQUALS(move, Figure::Move("e8g8"));
  TEST_END
}

} // unnamed namespace
