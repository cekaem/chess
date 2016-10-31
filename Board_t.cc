/* Component tests for class Board */

#include "Test.h"
#include "Board.h"

#include "Field.h"
#include "Figure.h"

#include <memory>
#include <utility>

namespace {

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

// Checks if Board::FieldNotEmptyException is properly thrown from Board::addFigure
TEST_PROCEDURE(test3) {
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

// Checks if Board::add/remove/getFigure works correctly
TEST_PROCEDURE(test7) {
  TEST_START
  Board board;
  const auto& figures = board.getFigures();
  VERIFY_IS_EQUAL(figures.size(), 0ul);
  Field field(Field::C, Field::ONE);
  VERIFY_IS_NULL(board.getFigure(field));
  board.addFigure(Figure::QUEEN, field, Figure::BLACK);
  VERIFY_EQUALS(figures.size(), 1ul);
  VERIFY_CONTAINS(figures, board.getFigure(field));
  board.removeFigure(field);
  VERIFY_IS_NULL(board.getFigure(field));
  VERIFY_IS_EQUAL(figures.size(), 0ul);
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

} // unnamed namespace


int main() {
  try {
    TEST("Field::Field throws Field::WrongFieldException", test1);
    TEST("Board::addFigure throws FieldNotEmptyException", test3);
    TEST("Board::removeFigure throws Board::NoFigureException", test5);
    TEST("Board::add/remove/move/getFigure works correctly", test7);
    TEST("Board::operator== works correctly", test8);
    TEST("Board::Board(const Board&) works correctly", test9);
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
