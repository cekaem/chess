/* Component tests for class Board */

#include "utils/Mock.h"
#include "utils/Test.h"
#include "Board.h"

#include "Field.h"
#include "Figure.h"

#include <memory>
#include <utility>

namespace {

void VerifyFigure(const Board& board, const char* field, Figure::Type type, Figure::Color color) {
  const Figure* figure = board.getFigure(Field(field));
  VERIFY_IS_NOT_NULL(figure);
  VERIFY_EQUALS(figure->getType(), type);
  VERIFY_EQUALS(figure->getColor(), color);
}

class BoardDrawerMock : public BoardDrawer {
 public:
  MOCK_CLASS(BoardDrawerMock)
  MOCK_METHOD3(onFigureAdded, void(Figure::Type, Figure::Color, Field));
  MOCK_METHOD1(onFigureRemoved, void(Field));
  MOCK_METHOD1(onFigureMoved, void(Figure::Move));
  MOCK_METHOD1(onGameFinished, void(Board::GameStatus));
};

class BoardDrawerOnlyGameFinishedMock : public BoardDrawer {
 public:
  MOCK_CLASS(BoardDrawerOnlyGameFinishedMock)
  void onFigureAdded(Figure::Type, Figure::Color, Field) override {}
  void onFigureRemoved(Field) override {}
  void onFigureMoved(Figure::Move) override {}
  MOCK_METHOD1(onGameFinished, void(Board::GameStatus));
};

TEST_PROCEDURE(FieldConstructorThrowsWrongFieldException) {
  TEST_START
  try {
    const Field field(Field::A, static_cast<Field::Number>(8));
  } catch (Field::WrongFieldException& exception) {
    VERIFY_IS_EQUAL(exception.letter, Field::A);
    VERIFY_IS_EQUAL(exception.number, static_cast<Field::Number>(8));
    try {
      const Field field(static_cast<Field::Letter>(-1), Field::TWO);
    } catch (Field::WrongFieldException& exception) {
      VERIFY_IS_EQUAL(exception.letter, static_cast<Field::Letter>(-1));
      VERIFY_IS_EQUAL(exception.number, Field::TWO);
      RETURN
    }
  }
  NOT_REACHED
  TEST_END
}

TEST_PROCEDURE(BoardMakeMoveThrowsNoFigureException) {
  TEST_START
  Field field(Field::C, Field::TWO);
  try {
    Board board;
    board.makeMove(field, Field(Field::D, Field::THREE));
  } catch(const Board::NoFigureException& exception) {
    VERIFY_EQUALS(field, exception.field_);
    RETURN
  }
  NOT_REACHED
  TEST_END
}

TEST_PROCEDURE(BoardMakeMoveThrowsIllegalMoveException) {
  TEST_START
  Field field1(Field::D, Field::TWO);
  Field field2(Field::C, Field::TWO);
  Board board;
  const Figure* bishop = board.addFigure(Figure::BISHOP, field1, Figure::WHITE);
  board.addFigure(Figure::KING, Field(Field::A, Field::ONE), Figure::WHITE);
  board.addFigure(Figure::KING, Field(Field::D, Field::EIGHT), Figure::BLACK);
  board.addFigure(Figure::ROOK, Field(Field::E, Field::EIGHT), Figure::BLACK);
  try {
    board.makeMove(field1, field2);
  } catch(const Board::IllegalMoveException& exception) {
    VERIFY_EQUALS(exception.field_, field2);
    VERIFY_EQUALS(exception.figure_, bishop);
    RETURN
  }
  NOT_REACHED
  TEST_END
}

TEST_PROCEDURE(BoardAddFigureThrowsFieldNotEmptyException) {
  TEST_START
  const Field field(Field::B, Field::FIVE);
  Board board;
  try {
    board.addFigure(Figure::BISHOP, field, Figure::BLACK);
    board.addFigure(Figure::KING, field, Figure::WHITE);
  } catch (Board::FieldNotEmptyException& exception) {
    VERIFY_IS_EQUAL(exception.field_, field);
    VERIFY_IS_EQUAL(exception.figure_, board.getFigure(field));
    RETURN
  }
  NOT_REACHED
  TEST_END
}

TEST_PROCEDURE(BoardRemoveFigureThrowsNoFigureException) {
  TEST_START
  const Field field(Field::H, Field::EIGHT);
  try {
    Board board;
    board.removeFigure(field);
  } catch (Board::NoFigureException& exception) {
    VERIFY_IS_EQUAL(exception.field_, field);
    RETURN
  }
  NOT_REACHED
  TEST_END
}

TEST_PROCEDURE(AddMoveRemoveGetFigureWorksCorrectly) {
  TEST_START
  Board board;
  VERIFY_IS_EQUAL(board.getFigures().size(), 0ul);
  Field field1(Field::C, Field::ONE);
  VERIFY_IS_NULL(board.getFigure(field1));
  board.addFigure(Figure::KING, Field("d3"), Figure::WHITE);
  board.addFigure(Figure::KING, Field("g7"), Figure::BLACK);
  const Figure* queen = board.addFigure(Figure::QUEEN, field1, Figure::BLACK);
  VERIFY(queen->getType() == Figure::QUEEN);
  VERIFY_EQUALS(queen->getPosition(), field1);
  VERIFY_EQUALS(board.getFigures().size(), 3ul);
  VERIFY_CONTAINS(board.getFigures(), queen);
  VERIFY_EQUALS(board.getFigure(field1), queen);
  Field field2(Field::F, Field::FOUR);
  const Figure* knight = board.addFigure(Figure::KNIGHT, field2, Figure::WHITE);
  auto white_figures = board.getFigures(Figure::WHITE);
  auto black_figures = board.getFigures(Figure::BLACK);
  VERIFY_CONTAINS(white_figures, knight);
  VERIFY_EQUALS(white_figures.size(), 2ul);
  VERIFY_CONTAINS(black_figures, queen);
  VERIFY_EQUALS(black_figures.size(), 2ul);
  board.setSideToMove(Figure::BLACK);
  board.makeMove(field1, field2);
  VERIFY_EQUALS(board.getFigure(field2), queen);
  VERIFY_EQUALS(queen->getPosition(), field2);
  VERIFY_CONTAINS(board.getFigures(), queen);
  board.removeFigure(field2);
  VERIFY_IS_NULL(board.getFigure(field2));
  VERIFY_IS_EQUAL(board.getFigures().size(), 2ul);
  TEST_END
}

TEST_PROCEDURE(BoardOperatorEqualsWorksCorrectly) {
  TEST_START
  Board board1;
  Board board2;
  VERIFY_TRUE(board1 == board2);
  VERIFY_TRUE(board1.setBoardFromFEN("b7/3rr2P/1K6/6qk/2Q1R3/pN6/1p3PP1/7N b Q d3 0 24"));
  VERIFY_TRUE(board1 != board2);
  VERIFY_TRUE(board2.setBoardFromFEN("B7/3rr2P/1K6/6qk/2Q1R3/pN6/1p3PP1/7N b Q d3 0 24"));
  VERIFY_TRUE(board1 != board2);
  VERIFY_TRUE(board2.setBoardFromFEN("b7/3rr2P/1K6/6qk/2Q1R3/pN6/1p3PP1/7N b - d3 0 24"));
  VERIFY_TRUE(board1 != board2);
  VERIFY_TRUE(board2.setBoardFromFEN("b7/3rr2P/1K6/6qk/2Q1R3/pN6/1p3PP1/7N b kq d3 0 24"));
  VERIFY_TRUE(board1 != board2);
  VERIFY_TRUE(board2.setBoardFromFEN("b7/3rr2P/1K6/6qk/2Q1R3/pN6/1p3PP1/7N b Q - 0 24"));
  VERIFY_TRUE(board1 != board2);
  VERIFY_TRUE(board2.setBoardFromFEN("b7/3rr2P/1K6/6qk/2Q1R3/pN6/1p3PP1/7N b Q e3 0 24"));
  VERIFY_TRUE(board1 != board2);
  VERIFY_TRUE(board2.setBoardFromFEN("b7/3rr2P/1K6/6qk/2Q1R3/pN6/1p3PP1/7N b Q d3 1 24"));
  VERIFY_TRUE(board1 != board2);
  VERIFY_TRUE(board2.setBoardFromFEN("b7/3rr2P/1K6/6qk/2Q1R3/pN6/1p3PP1/7N b Q d3 0 5"));
  VERIFY_TRUE(board1 != board2);
  VERIFY_TRUE(board2.setBoardFromFEN("b7/3rr2P/1K6/6qk/2Q1R3/pN6/1p3PP1/7N b Q d3 0 24"));
  VERIFY_TRUE(board1 == board2);
  TEST_END
}

TEST_PROCEDURE(BoardCopyConstructorWorksCorrectly) {
  TEST_START
  Board board1;
  VERIFY_TRUE(board1.setBoardFromFEN("r5n1/2k2p2/1p2p1p1/p2p4/P2N2p1/BP1bP3/3P1P2/R3K2r b KQ c3 0 32"));
  Board board2 = board1;
  VERIFY_EQUALS(board1, board2);
  TEST_END
}

TEST_PROCEDURE(BoardDrawerOnFigureAddedMovedRemovedAreCalled) {
  TEST_START
  BoardDrawerMock drawer;
  Board board;
  board.addBoardDrawer(&drawer);
  EXPECT_CALL(drawer, onFigureAdded(Figure::QUEEN, Figure::BLACK, Field("d4")));
  EXPECT_CALL(drawer, onFigureAdded(Figure::BISHOP, Figure::WHITE, Field("e5")));
  EXPECT_CALL(drawer, onFigureAdded(Figure::KING, Figure::WHITE, Field("c2")));
  EXPECT_CALL(drawer, onFigureAdded(Figure::KING, Figure::BLACK, Field("a8")));
  EXPECT_CALL(drawer, onFigureMoved(Figure::Move(Field("d4"), Field("e4"), true, false, Figure::Move::Castling::LAST, false, Figure::PAWN)));
  EXPECT_CALL(drawer, onFigureMoved(Figure::Move(Field("c2"), Field("c3"), false, false, Figure::Move::Castling::LAST, false, Figure::PAWN)));
  EXPECT_CALL(drawer, onFigureMoved(Figure::Move(Field("e4"), Field("e5"), true, false, Figure::Move::Castling::LAST, true, Figure::PAWN)));
  EXPECT_CALL(drawer, onFigureRemoved(Field("e5")));  // bishop beaten by queen
  board.addFigure(Figure::BISHOP, Field("e5"), Figure::WHITE);
  board.addFigure(Figure::QUEEN, Field("d4"), Figure::BLACK);
  board.addFigure(Figure::KING, Field("c2"), Figure::WHITE);
  board.addFigure(Figure::KING, Field("a8"), Figure::BLACK);
  board.setSideToMove(Figure::BLACK);
  VERIFY_EQUALS(board.makeMove(Field("d4"), Field("e4")), Board::GameStatus::NONE);
  VERIFY_EQUALS(board.makeMove(Field("c2"), Field("c3")), Board::GameStatus::NONE);
  VERIFY_EQUALS(board.makeMove(Field("e4"), Field("e5")), Board::GameStatus::NONE);
  TEST_END
}

TEST_PROCEDURE(BoardDrawerOnGameFinishedIsCalledCorrectly) {
  TEST_START
  BoardDrawerMock drawer;
  {
    Board board;
    board.addBoardDrawer(&drawer);
    EXPECT_CALL(drawer, onFigureAdded(Figure::KING, Figure::WHITE, Field("d3")));
    EXPECT_CALL(drawer, onFigureAdded(Figure::KING, Figure::BLACK, Field("a2")));
    EXPECT_CALL(drawer, onFigureAdded(Figure::QUEEN, Figure::BLACK, Field("e3")));
    EXPECT_CALL(drawer, onFigureMoved(
          Figure::Move(Field("d3"), Field("e3"), false, false, Figure::Move::Castling::LAST, true, Figure::PAWN)));
    EXPECT_CALL(drawer, onFigureRemoved(Field("e3")));
    EXPECT_CALL(drawer, onGameFinished(Board::GameStatus::DRAW));

    board.addFigure(Figure::KING, Field("d3"), Figure::WHITE);
    board.addFigure(Figure::KING, Field("a2"), Figure::BLACK);
    board.addFigure(Figure::QUEEN, Field("e3"), Figure::BLACK);

    VERIFY_EQUALS(board.makeMove(Field("d3"), Field("e3")), Board::GameStatus::DRAW);
  }
  {
    Board board;
    board.addBoardDrawer(&drawer);
    EXPECT_CALL(drawer, onFigureAdded(Figure::KING, Figure::WHITE, Field("a8")));
    EXPECT_CALL(drawer, onFigureAdded(Figure::KING, Figure::BLACK, Field("c7")));
    EXPECT_CALL(drawer, onFigureAdded(Figure::QUEEN, Figure::BLACK, Field("c5")));
    EXPECT_CALL(drawer, onFigureMoved(
          Figure::Move(Field("c5"), Field("a5"), true, true, Figure::Move::Castling::LAST, false, Figure::PAWN)));
    EXPECT_CALL(drawer, onGameFinished(Board::GameStatus::BLACK_WON));

    board.addFigure(Figure::KING, Field("a8"), Figure::WHITE);
    board.addFigure(Figure::KING, Field("c7"), Figure::BLACK);
    board.addFigure(Figure::QUEEN, Field("c5"), Figure::BLACK);
    board.setSideToMove(Figure::BLACK);

    VERIFY_EQUALS(board.makeMove(Field("c5"), Field("a5")), Board::GameStatus::BLACK_WON);
  }
  {
    Board board;
    board.addBoardDrawer(&drawer);
    EXPECT_CALL(drawer, onFigureAdded(Figure::KING, Figure::WHITE, Field("a8")));
    EXPECT_CALL(drawer, onFigureAdded(Figure::KING, Figure::BLACK, Field("c7")));
    EXPECT_CALL(drawer, onFigureAdded(Figure::QUEEN, Figure::BLACK, Field("c5")));
    EXPECT_CALL(drawer, onFigureMoved(
          Figure::Move(Field("c5"), Field("b6"), false, false, Figure::Move::Castling::LAST, false, Figure::PAWN)));
    EXPECT_CALL(drawer, onGameFinished(Board::GameStatus::DRAW));

    board.addFigure(Figure::KING, Field("a8"), Figure::WHITE);
    board.addFigure(Figure::KING, Field("c7"), Figure::BLACK);
    board.addFigure(Figure::QUEEN, Field("c5"), Figure::BLACK);
    board.setSideToMove(Figure::BLACK);

    VERIFY_EQUALS(board.makeMove(Field("c5"), Field("b6")), Board::GameStatus::DRAW);
  }
  TEST_END
}

TEST_PROCEDURE(BoardIsMoveValidWorksCorrectly) {
  TEST_START
  Board board;
  VERIFY_FALSE(board.isMoveValid(Field("d1"), Field("d2")));
  board.addFigure(Figure::KING, Field("d1"), Figure::WHITE);
  VERIFY_FALSE(board.isMoveValid(Field("d1"), Field("d2")));  // lack of black king
  board.addFigure(Figure::KING, Field("a3"), Figure::BLACK);
  VERIFY_FALSE(board.isMoveValid(Field("d1"), Field("d2")));  // drawing position, insufficient material
  board.addFigure(Figure::ROOK, Field("h3"), Figure::WHITE);
  VERIFY_FALSE(board.isMoveValid(Field("d1"), Field("d2")));  // black king is checked
  board.addFigure(Figure::PAWN, Field("b3"), Figure::BLACK);
  VERIFY_TRUE(board.isMoveValid(Field("d1"), Field("d2")));
  VERIFY_TRUE(board.isMoveValid(Field("h3"), Field("h2")));
  VERIFY_FALSE(board.isMoveValid(Field("b3"), Field("b4")));
  board.addFigure(Figure::BISHOP, Field("h5"), Figure::BLACK);
  VERIFY_FALSE(board.isMoveValid(Field("h3"), Field("h2")));
  TEST_END
}

TEST_PROCEDURE(BoardUndoLastReverisbleMoveWorksCorrectly) {
  TEST_START
  Board board;
  board.addFigure(Figure::KING, Field("e1"), Figure::WHITE);
  board.addFigure(Figure::ROOK, Field("h1"), Figure::WHITE);
  board.addFigure(Figure::PAWN, Field("b2"), Figure::WHITE);
  board.addFigure(Figure::PAWN, Field("g7"), Figure::WHITE);
  board.addFigure(Figure::KING, Field("d8"), Figure::BLACK);
  board.addFigure(Figure::QUEEN, Field("h8"), Figure::BLACK);
  board.addFigure(Figure::PAWN, Field("c4"), Figure::BLACK);

  Board copy = board;
  copy.makeMove("b2", "b4", Figure::PAWN, true);
  copy.makeMove("c4", "b3", Figure::PAWN, true);
  copy.makeMove("g7", "h8", Figure::BISHOP, true);
  copy.makeMove("d8", "c8", Figure::PAWN, true);
  copy.makeMove("e1", "g1", Figure::PAWN, true);
  VERIFY_TRUE(board != copy);
  copy.undoAllReversibleMoves();
  VERIFY_TRUE(board == copy);
  TEST_END
}

TEST_PROCEDURE(BoardCreateFENWorksCorrectly) {
  TEST_START
  {
    Board board;
    board.addFigure(Figure::KING, Field("e8"), Figure::BLACK);
    board.addFigure(Figure::ROOK, Field("h8"), Figure::BLACK);
    board.addFigure(Figure::ROOK, Field("b7"), Figure::BLACK);
    board.addFigure(Figure::PAWN, Field("h7"), Figure::BLACK);
    board.addFigure(Figure::PAWN, Field("g5"), Figure::WHITE);
    board.addFigure(Figure::QUEEN, Field("c6"), Figure::BLACK);
    board.addFigure(Figure::BISHOP, Field("a7"), Figure::WHITE);
    board.addFigure(Figure::KNIGHT, Field("d4"), Figure::BLACK);
    board.addFigure(Figure::ROOK, Field("f2"), Figure::WHITE);
    board.addFigure(Figure::ROOK, Field("a1"), Figure::WHITE);
    board.addFigure(Figure::KING, Field("e1"), Figure::WHITE);
    board.setSideToMove(Figure::BLACK);
    std::string fen = board.createFEN();
    VERIFY_STRINGS_EQUAL(fen.c_str(), "4k2r/Br5p/2q5/6P1/3n4/8/5R2/R3K3 b KQkq - 0 1");
    board.makeMove(Field("h7"), Field("h5"));
    fen = board.createFEN();
    VERIFY_STRINGS_EQUAL(fen.c_str(), "4k2r/Br6/2q5/6Pp/3n4/8/5R2/R3K3 w KQkq h6 0 2");
  }
  {
    Board board;
    board.setStandardBoard();
    std::string fen = board.createFEN();
    VERIFY_STRINGS_EQUAL(fen.c_str(), "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  }
  {
    Board board;
    board.addFigure(Figure::KING, Field("e8"), Figure::BLACK);
    board.addFigure(Figure::KING, Field("e1"), Figure::WHITE);
    board.addFigure(Figure::ROOK, Field("a8"), Figure::BLACK);
    board.addFigure(Figure::ROOK, Field("h8"), Figure::BLACK);
    board.addFigure(Figure::ROOK, Field("a1"), Figure::WHITE);
    board.addFigure(Figure::ROOK, Field("h1"), Figure::WHITE);
    board.setSideToMove(Figure::BLACK);
    std::string fen = board.createFEN();
    VERIFY_STRINGS_EQUAL(fen.c_str(), "r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1");
    board.makeMove(Field("e8"), Field("f8"));
    fen = board.createFEN();
    VERIFY_STRINGS_EQUAL(fen.c_str(), "r4k1r/8/8/8/8/8/8/R3K2R w KQ - 0 2");
    board.makeMove(Field("a1"), Field("a2"));
    board.setSideToMove(Figure::WHITE);
    board.makeMove(Field("a2"), Field("a1"));
    fen = board.createFEN();
    VERIFY_STRINGS_EQUAL(fen.c_str(), "r4k1r/8/8/8/8/8/8/R3K2R b K - 1 2");
  }
  TEST_END
}

TEST_PROCEDURE(BoardSetBoardFromFENWorksCorrectly) {
  TEST_START
  {
    Board board;
    std::string fen("7r/5pp1/2b5/7k/6n1/5B2/Q3K3/4R2q b Qkq c3 3 7");
    VERIFY_TRUE(board.setBoardFromFEN(fen));
    VERIFY_EQUALS(board.createFEN(), fen);
    VerifyFigure(board, "a2", Figure::QUEEN, Figure::WHITE);
    VerifyFigure(board, "e1", Figure::ROOK, Figure::WHITE);
    VerifyFigure(board, "f7", Figure::PAWN, Figure::BLACK);
    VerifyFigure(board, "f3", Figure::BISHOP, Figure::WHITE);
    VerifyFigure(board, "g7", Figure::PAWN, Figure::BLACK);
    VerifyFigure(board, "g4", Figure::KNIGHT, Figure::BLACK);
    VerifyFigure(board, "h1", Figure::QUEEN, Figure::BLACK);
    VerifyFigure(board, "h5", Figure::KING, Figure::BLACK);
    VerifyFigure(board, "h8", Figure::ROOK, Figure::BLACK);
    VerifyFigure(board, "c6", Figure::BISHOP, Figure::BLACK);
    VerifyFigure(board, "e2", Figure::KING, Figure::WHITE);
    VERIFY_EQUALS(board.getEnPassantFile(), Field::C);
    VERIFY_STRINGS_EQUAL(board.getFENForCastlings().c_str(), "Qkq");
    VERIFY_EQUALS(board.getHalfMoveClock(), 3u);
    VERIFY_EQUALS(board.getFullMoveNumber(), 7u);
  }
  TEST_END
}

TEST_PROCEDURE(BoardSetBoardFromFENDetectsInvalidFEN) {
  TEST_START
  {
    Board board;
    VERIFY_FALSE(board.setBoardFromFEN(""));
    VERIFY_FALSE(board.setBoardFromFEN("invalid_fen"));
    VERIFY_FALSE(board.setBoardFromFEN("7r/5pp1/2b5/7k/6n1/5B2/Q3K3/4R2q"));
    VERIFY_FALSE(board.setBoardFromFEN("7r/5dp1/2b5/7k/6n1/5B2/Q3K3/4R2q b Qkq c3 3 7"));
    VERIFY_FALSE(board.setBoardFromFEN("7r/5pp1/2b5/7k/6n1/5B2/Q3K3/4R2q b Qg c3 3 7"));
    VERIFY_FALSE(board.setBoardFromFEN("7r/5pp1/2b5/7k/6n1/5B2/Q3K3/4R2q b Qkq c4 3 7"));
    VERIFY_FALSE(board.setBoardFromFEN("7r/5pp1/2b5/7k/6n1/5B2/Q3K3/4R2q w Qkq c3 3 7"));
    VERIFY_FALSE(board.setBoardFromFEN("7r/5pp1/2b5/7k/6n1/5B2/Q3K3/4R2q b Qkq c6 3 7"));
    VERIFY_FALSE(board.setBoardFromFEN("7r/5pp1/2b5/7k/6n1/5B2/Q3K3/4R2q g Qkq c3 3 7"));
    VERIFY_FALSE(board.setBoardFromFEN("7r/5pp1/2b5/7k/6n1/5B2/Q3K3/4R2q b Qkq c3"));
    VERIFY_FALSE(board.setBoardFromFEN("7r/5pp1/2b5/7k/6n1/5B2/Q3K3/4R2q b Qkq c3 3 nan"));
  }
  TEST_END
}

TEST_PROCEDURE(BoardCorrectlyCountsHalfMoves) {
  TEST_START
  Board board;
  VERIFY_EQUALS(board.getHalfMoveClock(), 0u);
  VERIFY_TRUE(board.setBoardFromFEN("r7/3k3p/4q1P1/8/8/1R1K2PP/5N2/8 w KQkq - 0 0"));
  VERIFY_EQUALS(board.getHalfMoveClock(), 0u);
  board.makeMove(Field("d3"), Field("c3"));
  VERIFY_EQUALS(board.getHalfMoveClock(), 0u);
  board.makeMove(Field("d7"), Field("c7"));
  VERIFY_EQUALS(board.getHalfMoveClock(), 1u);
  board.makeMove(Field("c3"), Field("d3"));
  VERIFY_EQUALS(board.getHalfMoveClock(), 1u);
  board.makeMove(Field("c7"), Field("d7"));
  VERIFY_EQUALS(board.getHalfMoveClock(), 2u);
  board.makeMove(Field("f2"), Field("e4"));
  VERIFY_EQUALS(board.getHalfMoveClock(), 2u);
  board.makeMove(Field("e6"), Field("b3"));
  VERIFY_EQUALS(board.getHalfMoveClock(), 0u);
  board.makeMove(Field("e4"), Field("c3"));
  VERIFY_EQUALS(board.getHalfMoveClock(), 0u);
  board.makeMove(Field("a8"), Field("a7"));
  VERIFY_EQUALS(board.getHalfMoveClock(), 1u);
  board.makeMove(Field("g3"), Field("g4"));
  VERIFY_EQUALS(board.getHalfMoveClock(), 0u);
  TEST_END
}

TEST_PROCEDURE(GameIsDrawnAfterFiftyConsecutiveHalfMoves) {
  TEST_START
  Board board;
  BoardDrawerOnlyGameFinishedMock drawer;
  board.addBoardDrawer(&drawer);
  EXPECT_CALL(drawer, onGameFinished(Board::GameStatus::DRAW));
  VERIFY_TRUE(board.setBoardFromFEN("8/4k3/5q2/8/8/P7/4K3/8 w - - 0 0"));
  for (size_t i = 0; i < 24; ++i) {
    board.makeMove(Field("e2"), Field("e3"));
    board.makeMove(Field("f6"), Field("f7"));
    board.makeMove(Field("e3"), Field("e2"));
    board.makeMove(Field("f7"), Field("f6"));
  }
  VERIFY_EQUALS(board.getHalfMoveClock(), 48u);
  board.makeMove(Field("e2"), Field("e3"));
  board.makeMove(Field("f6"), Field("f7"));
  VERIFY_EQUALS(board.getHalfMoveClock(), 49u);
  board.makeMove(Field("e3"), Field("e2"));
  VERIFY_EQUALS(board.getHalfMoveClock(), 49u);
  board.makeMove(Field("f7"), Field("f6"));
  VERIFY_EQUALS(board.getHalfMoveClock(), 50u);
  TEST_END
}

TEST_PROCEDURE(FullMovesAreCorrectlyCounted) {
  TEST_START
  Board board;
  VERIFY_TRUE(board.setBoardFromFEN("8/4k2p/4r3/6P1/8/4R1K1/8/8 w KQkq - 0 34"));
  VERIFY_EQUALS(board.getFullMoveNumber(), 34u);
  board.makeMove(Field("e3"), Field("e6"));
  VERIFY_EQUALS(board.getFullMoveNumber(), 34u);
  board.makeMove(Field("e7"), Field("e6"));
  VERIFY_EQUALS(board.getFullMoveNumber(), 35u);
  board.makeMove(Field("g3"), Field("g4"));
  VERIFY_EQUALS(board.getFullMoveNumber(), 35u);
  board.makeMove(Field("h7"), Field("h5"));
  VERIFY_EQUALS(board.getFullMoveNumber(), 36u);
  board.makeMove(Field("g5"), Field("h6"));
  VERIFY_EQUALS(board.getFullMoveNumber(), 36u);
  board.makeMove(Field("e6"), Field("f7"));
  VERIFY_EQUALS(board.getFullMoveNumber(), 37u);
  VERIFY_STRINGS_EQUAL(board.createFEN().c_str(), "8/5k2/7P/8/6K1/8/8/8 w KQkq - 0 37");
  TEST_END
}

} // unnamed namespace
