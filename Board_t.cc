/* Component tests for class Board */

#include "Mock.h"
#include "Test.h"
#include "Board.h"

#include "Field.h"
#include "Figure.h"

#include <memory>
#include <utility>

namespace {

class BoardDrawerMock : public BoardDrawer {
 public:
  MOCK_CLASS(BoardDrawerMock)
  MOCK_METHOD3(onFigureAdded, void(Figure::Type, Figure::Color, Field));
  MOCK_METHOD1(onFigureRemoved, void(Field));
  MOCK_METHOD1(onFigureMoved, void(Figure::Move));
  MOCK_METHOD1(onGameFinished, void(Board::GameStatus));
};

// Checks if Field::WrongFieldException is properly thrown from Field::Field
TEST_PROCEDURE(test1) {
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

// Checks if Board::makeMove throws Board::NoFigureException
TEST_PROCEDURE(test2) {
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

// Checks if Board::makeMove throws Board::IllegalMoveException
TEST_PROCEDURE(test3) {
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

// Checks if Board::FieldNotEmptyException is properly thrown from Board::addFigure
TEST_PROCEDURE(test4) {
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

// Checks if Board::NoFigureException is properly thrown from Board::removeFigure
TEST_PROCEDURE(test5) {
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

// Checks if Board::add/move/remove/getFigure(s) works correctly
TEST_PROCEDURE(test7) {
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
  board.makeMove(field1, field2);
  VERIFY_EQUALS(board.getFigure(field2), queen);
  VERIFY_EQUALS(queen->getPosition(), field2);
  VERIFY_CONTAINS(board.getFigures(), queen);
  board.removeFigure(field2);
  VERIFY_IS_NULL(board.getFigure(field2));
  VERIFY_IS_EQUAL(board.getFigures().size(), 2ul);
  TEST_END
}

// Checks if Board::operator==/!= works correctly
TEST_PROCEDURE(test8) {
  TEST_START
  Board board1;
  Board board2;
  VERIFY(board1 == board2);
  Field field1(Field::A, Field::TWO);
  Field field2(Field::E, Field::SEVEN);
  Field field3(Field::E, Field::SIX);
  board1.addFigure(Figure::PAWN, field1, Figure::BLACK);
  VERIFY(board1 != board2);
  board1.addFigure(Figure::ROOK, field2, Figure::WHITE);
  board2.addFigure(Figure::PAWN, field1, Figure::BLACK);
  board2.addFigure(Figure::ROOK, field3, Figure::WHITE);
  VERIFY(board1 != board2);
  board2.removeFigure(field3);
  board2.addFigure(Figure::KING, field2, Figure::BLACK);
  VERIFY(board1 != board2);
  board2.removeFigure(field2);
  board2.addFigure(Figure::ROOK, field2, Figure::WHITE);
  VERIFY_EQUALS(board1, board2);
  TEST_END
}

// Checks if Board's copy constructor and Board::restoreFiguresPositions work correctly
TEST_PROCEDURE(test9) {
  TEST_START
  Board board1;
  Field field1(Field::D, Field::FOUR);
  Field field2(Field::B, Field::ONE);
  Field field3(Field::H, Field::SIX);
  board1.addFigure(Figure::KNIGHT, field1, Figure::WHITE);
  board1.addFigure(Figure::KNIGHT, field2, Figure::BLACK);
  board1.addFigure(Figure::QUEEN, field3, Figure::WHITE);
  Board board2 = board1;
  VERIFY_EQUALS(board1, board2);
  TEST_END
}

// Checks if BoardDrawer::onFigureAdded/Moved/Removed are called correctly
TEST_PROCEDURE(test10) {
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
  VERIFY_EQUALS(board.makeMove(Field("d4"), Field("e4")), Board::GameStatus::NONE);
  VERIFY_EQUALS(board.makeMove(Field("c2"), Field("c3")), Board::GameStatus::NONE);
  VERIFY_EQUALS(board.makeMove(Field("e4"), Field("e5")), Board::GameStatus::NONE);
  TEST_END
}

// Checks if BoardDrawer::onGameFinished is called correctly.
TEST_PROCEDURE(test11) {
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

    VERIFY_EQUALS(board.makeMove(Field("c5"), Field("b6")), Board::GameStatus::DRAW);
  }
  TEST_END
}

// Checks if Board::isMoveValid works correctly.
TEST_PROCEDURE(test12) {
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

// Checks if Board::undoLastReversibleMove works correctly.
TEST_PROCEDURE(test13) {
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

} // unnamed namespace


int main() {
  try {
    TEST("Field::Field throws Field::WrongFieldException", test1);
    TEST("Board::makeMove throws Board::NoFigureException", test2);
    TEST("Board::makeMove throws Board::IllegalMoveException", test3);
    TEST("Board::addFigure throws FieldNotEmptyException", test4);
    TEST("Board::removeFigure throws Board::NoFigureException", test5);
    TEST("Board::add/remove/move/getFigure(s) works correctly", test7);
    TEST("Board::operator== works correctly", test8);
    TEST("Board::Board(const Board&) works correctly", test9);
    TEST("BoardDrawer::onFigureAdded/Moved/Removed are called correctly", test10);
    TEST("BoardDrawer::onGameFinished is called correctly", test11);
    TEST("Board::isMoveValid works correctly", test12);
    TEST("Board::undoLastReversibleMove works correctly", test13);
  } catch (std::exception& except) {
    std::cerr << "Unexpected exception: " << except.what() << std::endl;
     return -1;
  }
  int failed_tests = Test::get_number_of_failed_tests();
  if (failed_tests > 0) {
    std::cout << failed_tests << " test(s) failed." << std::endl;
    return -2;
  }
  std::cout << "All tests passed." << std::endl;
  return 0;
}
