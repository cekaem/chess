/* Component tests for class Figure */

#include "Test.h"
#include "Figure.h"

#include "Board.h"

#include <utility>

namespace {

Figure::Move createMove(const Figure* figure, Field::Letter l, Field::Number n) {
  return std::make_pair(figure->getPosition(), Field(l, n));
}

// Checks if Pawn::calculatePossibleMoves returns proper moves
TEST_PROCEDURE(test1) {
  TEST_START
  {
    Board board;
    Field field(Field::C, Field::FIVE);
    const Figure* pawn = board.addFigure(Figure::PAWN, field, Figure::WHITE);
    auto moves = pawn->calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(pawn, Field::C, Field::SIX));
    VERIFY_EQUALS(moves.size(), 1lu);
  }
  {
    Board board;
    Field field(Field::H, Field::SEVEN);
    const Figure* pawn = board.addFigure(Figure::PAWN, field, Figure::BLACK);
    auto moves = pawn->calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(pawn, Field::H, Field::SIX));
    VERIFY_CONTAINS(moves, createMove(pawn, Field::H, Field::FIVE));
    VERIFY_EQUALS(moves.size(), 2lu);
  }
  {
    Board board;
    const Figure* pawn1 = board.addFigure(Figure::PAWN, Field(Field::E, Field::THREE), Figure::WHITE);
    board.addFigure(Figure::PAWN, Field(Field::F, Field::FOUR), Figure::BLACK);
    board.addFigure(Figure::PAWN, Field(Field::D, Field::FOUR), Figure::WHITE);
    board.addFigure(Figure::PAWN, Field(Field::E, Field::FOUR), Figure::BLACK);
    auto moves = pawn1->calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(pawn1, Field::F, Field::FOUR));
    VERIFY_EQUALS(moves.size(), 1lu);
  }
  {
    Board board;
    VERIFY_IS_NULL(board.getEnPassantPawn());
    const Figure* pawn1 = board.addFigure(Figure::PAWN, Field(Field::H, Field::TWO), Figure::WHITE);
    const Figure* pawn2 = board.addFigure(Figure::PAWN, Field(Field::G, Field::FOUR), Figure::BLACK);
    const Figure* pawn3 = board.addFigure(Figure::PAWN, Field(Field::C, Field::FIVE), Figure::WHITE);
    const Figure* pawn4 = board.addFigure(Figure::PAWN, Field(Field::D, Field::SEVEN), Figure::BLACK);
    VERIFY_IS_NULL(board.getEnPassantPawn());
    auto moves = pawn2->calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(pawn2, Field::G, Field::THREE));
    VERIFY_EQUALS(moves.size(), 1lu);
    board.moveFigure(Field(Field::H, Field::TWO), Field(Field::H, Field::FOUR));
    VERIFY_EQUALS(board.getEnPassantPawn(), static_cast<const Pawn*>(pawn1));
    moves = pawn2->calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(pawn2, Field::G, Field::THREE));
    VERIFY_CONTAINS(moves, createMove(pawn2, Field::H, Field::THREE));
    VERIFY_EQUALS(moves.size(), 2lu);
    board.moveFigure(Field(Field::D, Field::SEVEN), Field(Field::D, Field::FIVE));
    VERIFY_EQUALS(board.getEnPassantPawn(), static_cast<const Pawn*>(pawn4));
    moves = pawn2->calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(pawn2, Field::G, Field::THREE));
    VERIFY_EQUALS(moves.size(), 1lu);
    moves = pawn3->calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(pawn3, Field::C, Field::SIX));
    VERIFY_CONTAINS(moves, createMove(pawn3, Field::D, Field::SIX));
    VERIFY_EQUALS(moves.size(), 2lu);
  }
  TEST_END
}

// Checks if Knight::calculatePossibleMoves returns proper moves
TEST_PROCEDURE(test2) {
  TEST_START
  {
    Board board;
    const Figure* knight = board.addFigure(Figure::KNIGHT, Field(Field::E, Field::FOUR), Figure::WHITE);
    auto moves = knight->calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(knight, Field::D, Field::SIX));
    VERIFY_CONTAINS(moves, createMove(knight, Field::F, Field::SIX));
    VERIFY_CONTAINS(moves, createMove(knight, Field::G, Field::FIVE));
    VERIFY_CONTAINS(moves, createMove(knight, Field::G, Field::THREE));
    VERIFY_CONTAINS(moves, createMove(knight, Field::F, Field::TWO));
    VERIFY_CONTAINS(moves, createMove(knight, Field::D, Field::TWO));
    VERIFY_CONTAINS(moves, createMove(knight, Field::C, Field::THREE));
    VERIFY_CONTAINS(moves, createMove(knight, Field::C, Field::FIVE));
    VERIFY_EQUALS(moves.size(), 8lu);
  }
  {
    Board board;
    const Figure* knight = board.addFigure(Figure::KNIGHT, Field(Field::H, Field::EIGHT), Figure::BLACK);
    auto moves = knight->calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(knight, Field::F, Field::SEVEN));
    VERIFY_CONTAINS(moves, createMove(knight, Field::G, Field::SIX));
    VERIFY_EQUALS(moves.size(), 2lu);
  }
  {
    Board board;
    const Figure* knight = board.addFigure(Figure::KNIGHT, Field(Field::B, Field::ONE), Figure::BLACK);
    board.addFigure(Figure::KNIGHT, Field(Field::D, Field::TWO), Figure::WHITE);
    board.addFigure(Figure::PAWN, Field(Field::C, Field::THREE), Figure::BLACK);
    auto moves = knight->calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(knight, Field::A, Field::THREE));
    VERIFY_CONTAINS(moves, createMove(knight, Field::D, Field::TWO));
    VERIFY_EQUALS(moves.size(), 2lu);
  }
  TEST_END
}

// Checks if Bishop::calculatePossibleMoves returns proper moves
TEST_PROCEDURE(test3) {
  TEST_START
  {
    Board board;
    const Figure* bishop = board.addFigure(Figure::BISHOP, Field(Field::D, Field::FOUR), Figure::BLACK);
    auto moves = bishop->calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(bishop, Field::C, Field::THREE));
    VERIFY_CONTAINS(moves, createMove(bishop, Field::B, Field::TWO));
    VERIFY_CONTAINS(moves, createMove(bishop, Field::A, Field::ONE));
    VERIFY_CONTAINS(moves, createMove(bishop, Field::E, Field::THREE));
    VERIFY_CONTAINS(moves, createMove(bishop, Field::F, Field::TWO));
    VERIFY_CONTAINS(moves, createMove(bishop, Field::G, Field::ONE));
    VERIFY_CONTAINS(moves, createMove(bishop, Field::C, Field::FIVE));
    VERIFY_CONTAINS(moves, createMove(bishop, Field::B, Field::SIX));
    VERIFY_CONTAINS(moves, createMove(bishop, Field::A, Field::SEVEN));
    VERIFY_CONTAINS(moves, createMove(bishop, Field::E, Field::FIVE));
    VERIFY_CONTAINS(moves, createMove(bishop, Field::F, Field::SIX));
    VERIFY_CONTAINS(moves, createMove(bishop, Field::G, Field::SEVEN));
    VERIFY_CONTAINS(moves, createMove(bishop, Field::H, Field::EIGHT));
    VERIFY_EQUALS(moves.size(), 13lu);
  }
  {
    Board board;
    const Figure* bishop = board.addFigure(Figure::BISHOP, Field(Field::F, Field::TWO), Figure::WHITE);
    board.addFigure(Figure::KNIGHT, Field(Field::D, Field::FOUR), Figure::WHITE);
    board.addFigure(Figure::PAWN, Field(Field::G, Field::THREE), Figure::BLACK);
    auto moves = bishop->calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(bishop, Field::E, Field::ONE));
    VERIFY_CONTAINS(moves, createMove(bishop, Field::E, Field::THREE));
    VERIFY_CONTAINS(moves, createMove(bishop, Field::G, Field::ONE));
    VERIFY_CONTAINS(moves, createMove(bishop, Field::G, Field::THREE));
    VERIFY_EQUALS(moves.size(), 4lu);
  }
  TEST_END
}

// Checks if Rook::calculatePossibleMoves returns proper moves
TEST_PROCEDURE(test4) {
  TEST_START
  {
    Board board;
    const Figure* rook = board.addFigure(Figure::ROOK, Field(Field::E, Field::FIVE), Figure::WHITE);
    auto moves = rook->calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(rook, Field::D, Field::FIVE));
    VERIFY_CONTAINS(moves, createMove(rook, Field::C, Field::FIVE));
    VERIFY_CONTAINS(moves, createMove(rook, Field::B, Field::FIVE));
    VERIFY_CONTAINS(moves, createMove(rook, Field::A, Field::FIVE));
    VERIFY_CONTAINS(moves, createMove(rook, Field::E, Field::SIX));
    VERIFY_CONTAINS(moves, createMove(rook, Field::E, Field::SEVEN));
    VERIFY_CONTAINS(moves, createMove(rook, Field::E, Field::EIGHT));
    VERIFY_CONTAINS(moves, createMove(rook, Field::F, Field::FIVE));
    VERIFY_CONTAINS(moves, createMove(rook, Field::G, Field::FIVE));
    VERIFY_CONTAINS(moves, createMove(rook, Field::H, Field::FIVE));
    VERIFY_CONTAINS(moves, createMove(rook, Field::E, Field::FOUR));
    VERIFY_CONTAINS(moves, createMove(rook, Field::E, Field::THREE));
    VERIFY_CONTAINS(moves, createMove(rook, Field::E, Field::TWO));
    VERIFY_CONTAINS(moves, createMove(rook, Field::E, Field::ONE));
    VERIFY_EQUALS(moves.size(), 14lu);
  }
  {
    Board board;
    const Figure* rook = board.addFigure(Figure::ROOK, Field(Field::D, Field::ONE), Figure::BLACK);
    board.addFigure(Figure::BISHOP, Field(Field::D, Field::THREE), Figure::BLACK);
    board.addFigure(Figure::BISHOP, Field(Field::E, Field::ONE), Figure::WHITE);
    board.addFigure(Figure::KNIGHT, Field(Field::A, Field::ONE), Figure::WHITE);
    auto moves = rook->calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(rook, Field::C, Field::ONE));
    VERIFY_CONTAINS(moves, createMove(rook, Field::B, Field::ONE));
    VERIFY_CONTAINS(moves, createMove(rook, Field::D, Field::TWO));
    VERIFY_CONTAINS(moves, createMove(rook, Field::A, Field::ONE));
    VERIFY_CONTAINS(moves, createMove(rook, Field::E, Field::ONE));
    VERIFY_EQUALS(moves.size(), 5lu);
  }
  TEST_END
}

// Checks if Queen::calculatePossibleMoves returns proper moves
TEST_PROCEDURE(test5) {
  TEST_START
  {
    Board board;
    const Figure* queen = board.addFigure(Figure::QUEEN, Field(Field::E, Field::FIVE), Figure::BLACK);
    auto moves = queen->calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(queen, Field::D, Field::SIX));
    VERIFY_CONTAINS(moves, createMove(queen, Field::C, Field::SEVEN));
    VERIFY_CONTAINS(moves, createMove(queen, Field::B, Field::EIGHT));
    VERIFY_CONTAINS(moves, createMove(queen, Field::E, Field::SIX));
    VERIFY_CONTAINS(moves, createMove(queen, Field::E, Field::SEVEN));
    VERIFY_CONTAINS(moves, createMove(queen, Field::E, Field::EIGHT));
    VERIFY_CONTAINS(moves, createMove(queen, Field::F, Field::SIX));
    VERIFY_CONTAINS(moves, createMove(queen, Field::G, Field::SEVEN));
    VERIFY_CONTAINS(moves, createMove(queen, Field::F, Field::FIVE));
    VERIFY_CONTAINS(moves, createMove(queen, Field::G, Field::FIVE));
    VERIFY_CONTAINS(moves, createMove(queen, Field::H, Field::FIVE));
    VERIFY_CONTAINS(moves, createMove(queen, Field::F, Field::FOUR));
    VERIFY_CONTAINS(moves, createMove(queen, Field::G, Field::THREE));
    VERIFY_CONTAINS(moves, createMove(queen, Field::H, Field::TWO));
    VERIFY_CONTAINS(moves, createMove(queen, Field::E, Field::FOUR));
    VERIFY_CONTAINS(moves, createMove(queen, Field::E, Field::THREE));
    VERIFY_CONTAINS(moves, createMove(queen, Field::E, Field::TWO));
    VERIFY_CONTAINS(moves, createMove(queen, Field::E, Field::ONE));
    VERIFY_CONTAINS(moves, createMove(queen, Field::D, Field::FOUR));
    VERIFY_CONTAINS(moves, createMove(queen, Field::C, Field::THREE));
    VERIFY_CONTAINS(moves, createMove(queen, Field::B, Field::TWO));
    VERIFY_CONTAINS(moves, createMove(queen, Field::A, Field::ONE));
    VERIFY_CONTAINS(moves, createMove(queen, Field::D, Field::FIVE));
    VERIFY_CONTAINS(moves, createMove(queen, Field::C, Field::FIVE));
    VERIFY_CONTAINS(moves, createMove(queen, Field::B, Field::FIVE));
    VERIFY_CONTAINS(moves, createMove(queen, Field::A, Field::FIVE));
    VERIFY_EQUALS(moves.size(), 27lu);
  }
  {
    Board board;
    const Figure* queen = board.addFigure(Figure::QUEEN, Field(Field::H, Field::THREE), Figure::WHITE);
    board.addFigure(Figure::ROOK, Field(Field::G, Field::THREE), Figure::WHITE);
    board.addFigure(Figure::KNIGHT, Field(Field::H, Field::SIX), Figure::WHITE);
    board.addFigure(Figure::BISHOP, Field(Field::F, Field::FIVE), Figure::BLACK);
    board.addFigure(Figure::PAWN, Field(Field::H, Field::TWO), Figure::BLACK);
    auto moves = queen->calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(queen, Field::G, Field::FOUR));
    VERIFY_CONTAINS(moves, createMove(queen, Field::F, Field::FIVE));
    VERIFY_CONTAINS(moves, createMove(queen, Field::H, Field::FOUR));
    VERIFY_CONTAINS(moves, createMove(queen, Field::H, Field::FIVE));
    VERIFY_CONTAINS(moves, createMove(queen, Field::H, Field::TWO));
    VERIFY_CONTAINS(moves, createMove(queen, Field::G, Field::TWO));
    VERIFY_CONTAINS(moves, createMove(queen, Field::F, Field::ONE));
    VERIFY_EQUALS(moves.size(), 7lu);
  }
  TEST_END
}

// Checks if King::calculatePossibleMoves returns proper moves
TEST_PROCEDURE(test6) {
  TEST_START
  {
    Board board;
    const Figure* king = board.addFigure(Figure::KING, Field(Field::D, Field::FIVE), Figure::BLACK);
    auto moves = king->calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(king, Field::C, Field::FIVE));
    VERIFY_CONTAINS(moves, createMove(king, Field::C, Field::SIX));
    VERIFY_CONTAINS(moves, createMove(king, Field::D, Field::SIX));
    VERIFY_CONTAINS(moves, createMove(king, Field::E, Field::SIX));
    VERIFY_CONTAINS(moves, createMove(king, Field::E, Field::FIVE));
    VERIFY_CONTAINS(moves, createMove(king, Field::E, Field::FOUR));
    VERIFY_CONTAINS(moves, createMove(king, Field::D, Field::FOUR));
    VERIFY_CONTAINS(moves, createMove(king, Field::C, Field::FOUR));
    VERIFY_EQUALS(moves.size(), 8lu);
  }
  {
    Board board;
    const Figure* king = board.addFigure(Figure::KING, Field(Field::E, Field::THREE), Figure::WHITE);
    board.addFigure(Figure::BISHOP, Field(Field::E, Field::FOUR), Figure::WHITE);
    board.addFigure(Figure::KNIGHT, Field(Field::F, Field::FOUR), Figure::BLACK);
    board.addFigure(Figure::KING, Field(Field::G, Field::FIVE), Figure::BLACK);
    board.addFigure(Figure::ROOK, Field(Field::D, Field::FOUR), Figure::BLACK);
    board.addFigure(Figure::QUEEN, Field(Field::H, Field::ONE), Figure::BLACK);
    auto moves = king->calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(king, Field::F, Field::TWO));
    VERIFY_CONTAINS(moves, createMove(king, Field::D, Field::FOUR));
    VERIFY_EQUALS(moves.size(), 2lu);
  }
  {
    // Look if castlings are properly detected as valid moves
    Board board;
    const Figure* white_king = board.addFigure(Figure::KING, Field(Field::E, Field::ONE), Figure::WHITE);
    const Figure* black_king = board.addFigure(Figure::KING, Field(Field::E, Field::EIGHT), Figure::BLACK);
    auto moves = white_king->calculatePossibleMoves();
    VERIFY_DOES_NOT_CONTAIN(moves, createMove(white_king, Field::G, Field::ONE));
    VERIFY_DOES_NOT_CONTAIN(moves, createMove(white_king, Field::C, Field::ONE));
    board.addFigure(Figure::ROOK, Field(Field::A, Field::ONE), Figure::WHITE);
    board.addFigure(Figure::ROOK, Field(Field::H, Field::ONE), Figure::WHITE);
    board.addFigure(Figure::ROOK, Field(Field::A, Field::EIGHT), Figure::BLACK);
    board.addFigure(Figure::ROOK, Field(Field::H, Field::EIGHT), Figure::BLACK);
    moves = white_king->calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(white_king, Field::G, Field::ONE));
    VERIFY_CONTAINS(moves, createMove(white_king, Field::C, Field::ONE));
    moves = black_king->calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(black_king, Field::G, Field::EIGHT));
    VERIFY_CONTAINS(moves, createMove(black_king, Field::G, Field::EIGHT));
    board.addFigure(Figure::QUEEN, Field(Field::D, Field::FOUR), Figure::BLACK);
    moves = white_king->calculatePossibleMoves();
    VERIFY_DOES_NOT_CONTAIN(moves, createMove(black_king, Field::G, Field::ONE));
    VERIFY_DOES_NOT_CONTAIN(moves, createMove(black_king, Field::C, Field::ONE));
    board.addFigure(Figure::BISHOP, Field(Field::H, Field::FIVE), Figure::WHITE);
    moves = black_king->calculatePossibleMoves();
    VERIFY_DOES_NOT_CONTAIN(moves, createMove(black_king, Field::G, Field::EIGHT));
    VERIFY_DOES_NOT_CONTAIN(moves, createMove(black_king, Field::C, Field::EIGHT));
  }
  {
    Board board;
    const Figure* white_king = board.addFigure(Figure::KING, Field(Field::E, Field::ONE), Figure::WHITE);
    board.addFigure(Figure::KING, Field(Field::E, Field::EIGHT), Figure::BLACK);
    board.addFigure(Figure::ROOK, Field(Field::A, Field::ONE), Figure::WHITE);
    board.addFigure(Figure::ROOK, Field(Field::H, Field::ONE), Figure::WHITE);
    board.addFigure(Figure::ROOK, Field(Field::A, Field::EIGHT), Figure::BLACK);
    const Figure* black_rook = board.addFigure(Figure::ROOK, Field(Field::H, Field::EIGHT), Figure::BLACK);
    board.moveFigure(Field(Field::A, Field::ONE), Field(Field::A, Field::TWO));
    board.moveFigure(Field(Field::A, Field::TWO), Field(Field::A, Field::ONE));
    auto moves = white_king->calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(white_king, Field::G, Field::ONE));
    VERIFY_DOES_NOT_CONTAIN(moves, createMove(white_king, Field::C, Field::ONE));
    board.moveFigure(Field(Field::E, Field::ONE), Field(Field::E, Field::TWO));
    board.moveFigure(Field(Field::E, Field::TWO), Field(Field::E, Field::ONE));
    moves = white_king->calculatePossibleMoves();
    VERIFY_DOES_NOT_CONTAIN(moves, createMove(white_king, Field::C, Field::ONE));
    VERIFY_DOES_NOT_CONTAIN(moves, createMove(white_king, Field::G, Field::ONE));
    board.moveFigure(Field(Field::E, Field::EIGHT), Field(Field::G, Field::EIGHT));
    VERIFY_EQUALS(black_rook->getPosition(), Field(Field::F, Field::EIGHT));
    VERIFY_EQUALS(board.getFigure(Field(Field::F, Field::EIGHT)), black_rook);
  }
  TEST_END
}

// Checks if King::isChecked and King::isCheckmated work correctly
TEST_PROCEDURE(test7) {
  TEST_START
  {
    Board board;
    const King* king = static_cast<const King*>(board.addFigure(Figure::KING, Field(Field::E, Field::THREE), Figure::WHITE));
    VERIFY_FALSE(king->isChecked());
    VERIFY_FALSE(king->isCheckmated());
    board.addFigure(Figure::ROOK, Field(Field::A, Field::FOUR), Figure::BLACK);
    VERIFY_FALSE(king->isChecked());
    VERIFY_FALSE(king->isCheckmated());
    board.addFigure(Figure::QUEEN, Field(Field::D, Field::TWO), Figure::BLACK);
    VERIFY_TRUE(king->isChecked());
    VERIFY_FALSE(king->isCheckmated());
    board.addFigure(Figure::KNIGHT, Field(Field::G, Field::FIVE), Figure::BLACK);
    VERIFY_TRUE(king->isChecked());
    VERIFY_FALSE(king->isCheckmated());
    board.addFigure(Figure::PAWN, Field(Field::C, Field::THREE), Figure::BLACK);
    VERIFY_TRUE(king->isChecked());
    VERIFY_TRUE(king->isCheckmated());
  }
  {
    Board board;
    const King* king = static_cast<const King*>(board.addFigure(Figure::KING, Field(Field::E, Field::ONE), Figure::WHITE));
    const Figure* queen = board.addFigure(Figure::QUEEN, Field(Field::D, Field::ONE), Figure::WHITE);
    const Figure* pawn = board.addFigure(Figure::PAWN, Field(Field::D, Field::TWO), Figure::WHITE);
    const Figure* knight = board.addFigure(Figure::KNIGHT, Field(Field::E, Field::TWO), Figure::WHITE);
    const Figure* bishop = board.addFigure(Figure::BISHOP, Field(Field::F, Field::ONE), Figure::WHITE);
    board.addFigure(Figure::QUEEN, Field(Field::H, Field::FOUR), Figure::BLACK);
    VERIFY_TRUE(king->isChecked());
    VERIFY_FALSE(king->isCheckmated());
    VERIFY_TRUE(king->calculatePossibleMoves().empty());
    VERIFY_TRUE(queen->calculatePossibleMoves().empty());
    VERIFY_TRUE(pawn->calculatePossibleMoves().empty());
    VERIFY_TRUE(bishop->calculatePossibleMoves().empty());
    auto moves = knight->calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(knight, Field::G, Field::THREE));
    VERIFY_EQUALS(moves.size(), 1lu);
  }
  TEST_END
}

//Checks if figures does not unveil their king
TEST_PROCEDURE(test8) {
  TEST_START
  Board board;
  board.addFigure(Figure::KING, Field(Field::D, Field::FOUR), Figure::WHITE);
  const Figure* pawn = board.addFigure(Figure::PAWN, Field(Field::C, Field::FOUR), Figure::WHITE);
  const Figure* knight = board.addFigure(Figure::KNIGHT, Field(Field::C, Field::THREE), Figure::WHITE);
  const Figure* bishop = board.addFigure(Figure::BISHOP, Field(Field::E, Field::FIVE), Figure::WHITE);
  const Figure* rook = board.addFigure(Figure::ROOK, Field(Field::D, Field::THREE), Figure::WHITE);
  board.addFigure(Figure::ROOK, Field(Field::A, Field::TWO), Figure::WHITE);
  const Figure* queen = board.addFigure(Figure::QUEEN, Field(Field::E, Field::FOUR), Figure::WHITE);
  board.addFigure(Figure::KING, Field(Field::H, Field::TWO), Figure::BLACK);
  board.addFigure(Figure::BISHOP, Field(Field::A, Field::ONE), Figure::BLACK);
  board.addFigure(Figure::BISHOP, Field(Field::F, Field::SIX), Figure::BLACK);
  board.addFigure(Figure::ROOK, Field(Field::A, Field::FOUR), Figure::BLACK);
  board.addFigure(Figure::ROOK, Field(Field::D, Field::TWO), Figure::BLACK);
  board.addFigure(Figure::QUEEN, Field(Field::F, Field::FOUR), Figure::BLACK);
  auto moves = pawn->calculatePossibleMoves();
  VERIFY_TRUE(moves.empty());
  moves = knight->calculatePossibleMoves();
  VERIFY_TRUE(moves.empty());
  moves = bishop->calculatePossibleMoves();
  VERIFY_CONTAINS(moves, createMove(bishop, Field::F, Field::SIX));
  VERIFY_EQUALS(moves.size(), 1lu);
  moves = rook->calculatePossibleMoves();
  VERIFY_CONTAINS(moves, createMove(rook, Field::D, Field::TWO));
  VERIFY_EQUALS(moves.size(), 1lu);
  moves = queen->calculatePossibleMoves();
  VERIFY_CONTAINS(moves, createMove(queen, Field::F, Field::FOUR));
  VERIFY_EQUALS(moves.size(), 1lu);
  TEST_END
}

// Checks if King::isStalemated works properly
TEST_PROCEDURE(test9) {
  TEST_START
  Board board;
  const King* king = static_cast<const King*>(board.addFigure(Figure::KING, Field(Field::B, Field::ONE), Figure::WHITE));
  VERIFY_FALSE(king->isStalemated());
  board.addFigure(Figure::ROOK, Field(Field::A, Field::EIGHT), Figure::BLACK);
  VERIFY_FALSE(king->isStalemated());
  board.addFigure(Figure::ROOK, Field(Field::C, Field::TWO), Figure::BLACK);
  VERIFY_FALSE(king->isStalemated());
  board.addFigure(Figure::KNIGHT, Field(Field::E, Field::THREE), Figure::BLACK);
  VERIFY_TRUE(king->isStalemated());
  board.addFigure(Figure::PAWN, Field(Field::H, Field::TWO), Figure::WHITE);
  VERIFY_FALSE(king->isStalemated());
  TEST_END
}

} // unnamed namespace


int main() {
  try {
    TEST("Pawn::calculatePossibleMoves returns proper moves", test1);
    TEST("Knight::calculatePossibleMoves returns proper moves", test2);
    TEST("Bishop::calculatePossibleMoves returns proper moves", test3);
    TEST("Rook::calculatePossibleMoves returns proper moves", test4);
    TEST("Queen::calculatePossibleMoves returns proper moves", test5);
    TEST("King::calculatePossibleMoves returns proper moves", test6);
    TEST("King::isChecked and King::isCheckmated works properly", test7);
    TEST("Figures does not unveil their king", test8);
    TEST("King::isStalemated works properly", test9);
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
