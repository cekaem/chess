/* Component tests for class Board */

#include "Test.h"
#include "Board.h"

#include <memory>
#include <utility>

class Figure {
 public:
  Figure(int id) : id_(id) {}
  const int id_;
};

namespace {

// Checks if Board::WrongFieldException is properly thrown from Board::addFigure
TEST_PROCEDURE(test1) {
  TEST_START
  const Board::Field field = std::make_pair(Board::A, static_cast<Board::Number>(8));
  try {
    Board board;
    auto figure = std::make_unique<Figure>(5);
    board.addFigure(figure.get(), field);
  } catch (Board::WrongFieldException& exception) {
    VERIFY_IS_EQUAL(exception.field_, field);
    RETURN
  }
  NOT_REACHED
  TEST_END
}

// Checks if Board::NoFigureException is properly thrown from Board::addFigure
TEST_PROCEDURE(test2) {
  TEST_START
  const Board::Field field = std::make_pair(Board::B, Board::THREE);
  try {
    Board board;
    board.addFigure(nullptr, field);
  } catch (Board::NoFigureException& exception) {
    VERIFY_IS_EQUAL(exception.field_, field);
    RETURN
  }
  NOT_REACHED
  TEST_END
}

// Checks if Board::FieldNotEmptyException is properly thrown from Board::addFigure
TEST_PROCEDURE(test3) {
  TEST_START
  const Board::Field field = std::make_pair(Board::B, Board::FIVE);
  auto figure = std::make_unique<Figure>(12);
  try {
    Board board;
    board.addFigure(figure.get(), field);
    auto new_figure = std::make_unique<Figure>(15);
    board.addFigure(new_figure.get(), field);
  } catch (Board::FieldNotEmptyException& exception) {
    VERIFY_IS_EQUAL(exception.field_, field);
    VERIFY_IS_EQUAL(exception.figure_, const_cast<const Figure*>(figure.get()));
    RETURN
  }
  NOT_REACHED
  TEST_END
}

// Checks if Board::WrongFieldException is properly thrown from Board::removeFigure
TEST_PROCEDURE(test4) {
  TEST_START
  const Board::Field field = std::make_pair(static_cast<Board::Letter>(8), Board::THREE);
  try {
    Board board;
    board.removeFigure(field);
  } catch (Board::WrongFieldException& exception) {
    VERIFY_IS_EQUAL(exception.field_, field);
    RETURN
  }
  NOT_REACHED
  TEST_END
}

// Checks if Board::NoFigureException is properly thrown from Board::removeFigure
TEST_PROCEDURE(test5) {
  TEST_START
  const Board::Field field = std::make_pair(Board::H, Board::EIGHT);
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

// Checks if Board::WrongFieldException is properly thrown from Board::getFigure
TEST_PROCEDURE(test6) {
  TEST_START
  const Board::Field field = std::make_pair(Board::E, static_cast<Board::Number>(-1));
  try {
    Board board;
    board.getFigure(field);
  } catch (Board::WrongFieldException& exception) {
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
  Board::Field field = std::make_pair(Board::C, Board::ONE);
  VERIFY_IS_NULL(board.getFigure(field));
  auto figure_ptr = std::make_unique<Figure>(7);
  const Figure* figure = figure_ptr.get();
  board.addFigure(figure, field);
  VERIFY_IS_EQUAL(board.getFigure(field), figure);
  const Figure* old_figure = board.removeFigure(field);
  VERIFY_IS_EQUAL(old_figure, figure);
  VERIFY_IS_NULL(board.getFigure(field));
  TEST_END
}

} // unnamed namespace


int main() {
  try {
    TEST("Board::addFigure throws Board::WrongFieldException", test1);
    TEST("Board::addFigure throws Board::NoFigureException", test2);
    TEST("Board::addFigure throws FieldNotEmptyException", test3);
    TEST("Board::removeFigure throws Board::WrongFieldException", test4);
    TEST("Board::removeFigure throws Board::NoFigureException", test5);
    TEST("Board::getFigure throws Board::WrongFieldException", test6);
    TEST("Board::add/remove/move/getFigure works correctly", test7);
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
