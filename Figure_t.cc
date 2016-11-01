/* Component tests for class Figure */

#include "Test.h"
#include "Figure.h"

#include "Board.h"

#include <utility>

namespace {

Figure::Move createMove(Field::Letter l, Field::Number n, const Figure* f) {
  return std::make_pair(Field(l, n), f);
}

// Checks if Pawn::calculatePossibleMoves returns proper moves
TEST_PROCEDURE(test1) {
  TEST_START
  {
    Board board;
    Field field(Field::C, Field::FIVE);
    const Figure* pawn = board.addFigure(Figure::PAWN, field, Figure::WHITE);
    auto moves = pawn->calculatePossibleMoves();
		VERIFY_CONTAINS(moves, createMove(Field::C, Field::SIX, nullptr));
    VERIFY_EQUALS(moves.size(), 1lu);
  }
  {
    Board board;
    Field field(Field::H, Field::SEVEN);
    const Figure* pawn = board.addFigure(Figure::PAWN, field, Figure::BLACK);
    auto moves = pawn->calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(Field::H, Field::SIX, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::H, Field::FIVE, nullptr));
    VERIFY_EQUALS(moves.size(), 2lu);
  }
  {
    Board board;
    const Figure* pawn1 = board.addFigure(Figure::PAWN, Field(Field::E, Field::THREE), Figure::WHITE);
    const Figure* pawn2 = board.addFigure(Figure::PAWN, Field(Field::F, Field::FOUR), Figure::BLACK);
    board.addFigure(Figure::PAWN, Field(Field::D, Field::FOUR), Figure::WHITE);
    board.addFigure(Figure::PAWN, Field(Field::E, Field::FOUR), Figure::BLACK);
    auto moves = pawn1->calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(Field::F, Field::FOUR, pawn2));
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
    VERIFY_CONTAINS(moves, createMove(Field::G, Field::THREE, nullptr));
    VERIFY_EQUALS(moves.size(), 1lu);
    board.moveFigure(Field(Field::H, Field::TWO), Field(Field::H, Field::FOUR));
    VERIFY_EQUALS(board.getEnPassantPawn(), pawn1);
    moves = pawn2->calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(Field::G, Field::THREE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::H, Field::THREE, pawn1));
    VERIFY_EQUALS(moves.size(), 2lu);
    board.moveFigure(Field(Field::D, Field::SEVEN), Field(Field::D, Field::FIVE));
    VERIFY_EQUALS(board.getEnPassantPawn(), pawn4);
    moves = pawn2->calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(Field::G, Field::THREE, nullptr));
    VERIFY_EQUALS(moves.size(), 1lu);
    moves = pawn3->calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(Field::C, Field::SIX, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::D, Field::SIX, pawn4));
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
    VERIFY_CONTAINS(moves, createMove(Field::D, Field::SIX, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::F, Field::SIX, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::G, Field::FIVE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::G, Field::THREE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::F, Field::TWO, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::D, Field::TWO, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::C, Field::THREE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::C, Field::FIVE, nullptr));
    VERIFY_EQUALS(moves.size(), 8lu);
  }
  {
    Board board;
    const Figure* knight = board.addFigure(Figure::KNIGHT, Field(Field::H, Field::EIGHT), Figure::BLACK);
    auto moves = knight->calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(Field::F, Field::SEVEN, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::G, Field::SIX, nullptr));
    VERIFY_EQUALS(moves.size(), 2lu);
  }
  {
    Board board;
    const Figure* knight = board.addFigure(Figure::KNIGHT, Field(Field::B, Field::ONE), Figure::BLACK);
    const Figure* enemy_knight = board.addFigure(Figure::KNIGHT, Field(Field::D, Field::TWO), Figure::WHITE);
    board.addFigure(Figure::PAWN, Field(Field::C, Field::THREE), Figure::BLACK);
    auto moves = knight->calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(Field::A, Field::THREE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::D, Field::TWO, enemy_knight));
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
    VERIFY_CONTAINS(moves, createMove(Field::C, Field::THREE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::B, Field::TWO, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::A, Field::ONE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::E, Field::THREE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::F, Field::TWO, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::G, Field::ONE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::C, Field::FIVE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::B, Field::SIX, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::A, Field::SEVEN, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::E, Field::FIVE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::F, Field::SIX, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::G, Field::SEVEN, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::H, Field::EIGHT, nullptr));
    VERIFY_EQUALS(moves.size(), 13lu);
  }
  {
    Board board;
    const Figure* bishop = board.addFigure(Figure::BISHOP, Field(Field::F, Field::TWO), Figure::WHITE);
    board.addFigure(Figure::KNIGHT, Field(Field::D, Field::FOUR), Figure::WHITE);
    const Figure* enemy_pawn = board.addFigure(Figure::PAWN, Field(Field::G, Field::THREE), Figure::BLACK);
    auto moves = bishop->calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(Field::E, Field::ONE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::E, Field::THREE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::G, Field::ONE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::G, Field::THREE, enemy_pawn));
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
    VERIFY_CONTAINS(moves, createMove(Field::D, Field::FIVE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::C, Field::FIVE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::B, Field::FIVE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::A, Field::FIVE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::E, Field::SIX, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::E, Field::SEVEN, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::E, Field::EIGHT, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::F, Field::FIVE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::G, Field::FIVE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::H, Field::FIVE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::E, Field::FOUR, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::E, Field::THREE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::E, Field::TWO, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::E, Field::ONE, nullptr));
    VERIFY_EQUALS(moves.size(), 14lu);
  }
  {
    Board board;
    const Figure* rook = board.addFigure(Figure::ROOK, Field(Field::D, Field::ONE), Figure::BLACK);
    board.addFigure(Figure::BISHOP, Field(Field::D, Field::THREE), Figure::BLACK);
    const Figure* enemy_bishop = board.addFigure(Figure::BISHOP, Field(Field::E, Field::ONE), Figure::WHITE);
    const Figure* enemy_knight = board.addFigure(Figure::KNIGHT, Field(Field::A, Field::ONE), Figure::WHITE);
    auto moves = rook->calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(Field::C, Field::ONE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::B, Field::ONE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::D, Field::TWO, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::A, Field::ONE, enemy_knight));
    VERIFY_CONTAINS(moves, createMove(Field::E, Field::ONE, enemy_bishop));
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
    VERIFY_CONTAINS(moves, createMove(Field::D, Field::SIX, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::C, Field::SEVEN, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::B, Field::EIGHT, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::E, Field::SIX, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::E, Field::SEVEN, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::E, Field::EIGHT, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::F, Field::SIX, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::G, Field::SEVEN, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::F, Field::FIVE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::G, Field::FIVE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::H, Field::FIVE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::F, Field::FOUR, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::G, Field::THREE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::H, Field::TWO, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::E, Field::FOUR, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::E, Field::THREE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::E, Field::TWO, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::E, Field::ONE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::D, Field::FOUR, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::C, Field::THREE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::B, Field::TWO, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::A, Field::ONE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::D, Field::FIVE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::C, Field::FIVE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::B, Field::FIVE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::A, Field::FIVE, nullptr));
    VERIFY_EQUALS(moves.size(), 27lu);
  }
  {
    Board board;
    const Figure* queen = board.addFigure(Figure::QUEEN, Field(Field::H, Field::THREE), Figure::WHITE);
    board.addFigure(Figure::ROOK, Field(Field::G, Field::THREE), Figure::WHITE);
    board.addFigure(Figure::KNIGHT, Field(Field::H, Field::SIX), Figure::WHITE);
    const Figure* enemy_bishop = board.addFigure(Figure::BISHOP, Field(Field::F, Field::FIVE), Figure::BLACK);
    const Figure* enemy_pawn = board.addFigure(Figure::PAWN, Field(Field::H, Field::TWO), Figure::BLACK);
    auto moves = queen->calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(Field::G, Field::FOUR, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::F, Field::FIVE, enemy_bishop));
    VERIFY_CONTAINS(moves, createMove(Field::H, Field::FOUR, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::H, Field::FIVE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::H, Field::TWO, enemy_pawn));
    VERIFY_CONTAINS(moves, createMove(Field::G, Field::TWO, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::F, Field::ONE, nullptr));
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
    VERIFY_CONTAINS(moves, createMove(Field::C, Field::FIVE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::C, Field::SIX, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::D, Field::SIX, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::E, Field::SIX, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::E, Field::FIVE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::E, Field::FOUR, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::D, Field::FOUR, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::C, Field::FOUR, nullptr));
    VERIFY_EQUALS(moves.size(), 8lu);
  }
  {
    Board board;
    const Figure* king = board.addFigure(Figure::KING, Field(Field::E, Field::THREE), Figure::WHITE);
    board.addFigure(Figure::BISHOP, Field(Field::E, Field::FOUR), Figure::WHITE);
    board.addFigure(Figure::KNIGHT, Field(Field::F, Field::FOUR), Figure::BLACK);
    board.addFigure(Figure::KING, Field(Field::G, Field::FIVE), Figure::BLACK);
    const Figure* enemy_rook = board.addFigure(Figure::ROOK, Field(Field::D, Field::FOUR), Figure::BLACK);
    board.addFigure(Figure::QUEEN, Field(Field::H, Field::ONE), Figure::BLACK);
    auto moves = king->calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(Field::F, Field::TWO, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::D, Field::FOUR, enemy_rook));
    VERIFY_EQUALS(moves.size(), 2lu);
  }
  {
    // Look if castlings are properly detected as valid moves
    Board board;
    const Figure* white_king = board.addFigure(Figure::KING, Field(Field::E, Field::ONE), Figure::WHITE);
    const Figure* black_king = board.addFigure(Figure::KING, Field(Field::E, Field::EIGHT), Figure::BLACK);
    auto moves = white_king->calculatePossibleMoves();
    VERIFY_DOES_NOT_CONTAIN(moves, createMove(Field::G, Field::ONE, nullptr));
    VERIFY_DOES_NOT_CONTAIN(moves, createMove(Field::C, Field::ONE, nullptr));
    board.addFigure(Figure::ROOK, Field(Field::A, Field::ONE), Figure::WHITE);
    board.addFigure(Figure::ROOK, Field(Field::H, Field::ONE), Figure::WHITE);
    board.addFigure(Figure::ROOK, Field(Field::A, Field::EIGHT), Figure::BLACK);
    board.addFigure(Figure::ROOK, Field(Field::H, Field::EIGHT), Figure::BLACK);
    moves = white_king->calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(Field::G, Field::ONE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::C, Field::ONE, nullptr));
    moves = black_king->calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(Field::G, Field::EIGHT, nullptr));
    VERIFY_CONTAINS(moves, createMove(Field::G, Field::EIGHT, nullptr));
    board.addFigure(Figure::QUEEN, Field(Field::D, Field::FOUR), Figure::BLACK);
    moves = white_king->calculatePossibleMoves();
    VERIFY_DOES_NOT_CONTAIN(moves, createMove(Field::G, Field::ONE, nullptr));
    VERIFY_DOES_NOT_CONTAIN(moves, createMove(Field::C, Field::ONE, nullptr));
    board.addFigure(Figure::BISHOP, Field(Field::H, Field::FIVE), Figure::WHITE);
    moves = black_king->calculatePossibleMoves();
    VERIFY_DOES_NOT_CONTAIN(moves, createMove(Field::G, Field::EIGHT, nullptr));
    VERIFY_DOES_NOT_CONTAIN(moves, createMove(Field::C, Field::EIGHT, nullptr));
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
    VERIFY_CONTAINS(moves, createMove(Field::G, Field::ONE, nullptr));
    VERIFY_DOES_NOT_CONTAIN(moves, createMove(Field::C, Field::ONE, nullptr));
    board.moveFigure(Field(Field::E, Field::ONE), Field(Field::E, Field::TWO));
    board.moveFigure(Field(Field::E, Field::TWO), Field(Field::E, Field::ONE));
    moves = white_king->calculatePossibleMoves();
    VERIFY_DOES_NOT_CONTAIN(moves, createMove(Field::C, Field::ONE, nullptr));
    VERIFY_DOES_NOT_CONTAIN(moves, createMove(Field::G, Field::ONE, nullptr));
    board.moveFigure(Field(Field::E, Field::EIGHT), Field(Field::G, Field::EIGHT));
    VERIFY_EQUALS(black_rook->getPosition(), Field(Field::F, Field::EIGHT));
    VERIFY_EQUALS(board.getFigure(Field(Field::F, Field::EIGHT)), black_rook);
  }
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
