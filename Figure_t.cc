/* Component tests for class Figure */

#include "Test.h"
#include "Figure.h"

#include "Board.h"

#include <utility>

namespace {

Figure::Move createMove(Board::Letter l, Board::Number n, const Figure* f) {
  return std::make_pair(std::make_pair(l, n), f);
}

// Checks if Pawn::calculatePossibleMoves returns proper moves
TEST_PROCEDURE(test1) {
  TEST_START
  {
    Board board;
    Board::Field field = std::make_pair(Board::C, Board::FIVE);
    Pawn pawn(board, field, Figure::WHITE);
    auto moves = pawn.calculatePossibleMoves();
		VERIFY_CONTAINS(moves, createMove(Board::C, Board::SIX, nullptr));
    VERIFY_EQUALS(moves.size(), 1lu);
  }
  {
    Board board;
    Board::Field field = std::make_pair(Board::H, Board::SEVEN);
    Pawn pawn(board, field, Figure::BLACK);
    auto moves = pawn.calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(Board::H, Board::SIX, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::H, Board::FIVE, nullptr));
    VERIFY_EQUALS(moves.size(), 2lu);
  }
  {
    Board board;
    Pawn pawn1(board, std::make_pair(Board::E, Board::THREE), Figure::WHITE);
    Pawn pawn2(board, std::make_pair(Board::F, Board::FOUR), Figure::BLACK);
    Pawn pawn3(board, std::make_pair(Board::D, Board::FOUR), Figure::WHITE);
    Pawn pawn4(board, std::make_pair(Board::E, Board::FOUR), Figure::BLACK);
    auto moves = pawn1.calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(Board::F, Board::FOUR, &pawn2));
    VERIFY_EQUALS(moves.size(), 1lu);
  }
  {
    Board board;
    VERIFY_IS_NULL(board.getEnPassantPawn());
    Pawn pawn1(board, std::make_pair(Board::H, Board::TWO), Figure::WHITE);
    Pawn pawn2(board, std::make_pair(Board::G, Board::FOUR), Figure::BLACK);
    Pawn pawn3(board, std::make_pair(Board::C, Board::FIVE), Figure::WHITE);
    Pawn pawn4(board, std::make_pair(Board::D, Board::SEVEN), Figure::BLACK);
    VERIFY_IS_NULL(board.getEnPassantPawn());
    auto moves = pawn2.calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(Board::G, Board::THREE, nullptr));
    VERIFY_EQUALS(moves.size(), 1lu);
    pawn1.move(std::make_pair(Board::H, Board::FOUR));
    VERIFY_EQUALS(board.getEnPassantPawn(), const_cast<const Pawn*>(&pawn1));
    moves = pawn2.calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(Board::G, Board::THREE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::H, Board::THREE, &pawn1));
    VERIFY_EQUALS(moves.size(), 2lu);
    pawn4.move(std::make_pair(Board::D, Board::FIVE));
    VERIFY_EQUALS(board.getEnPassantPawn(), const_cast<const Pawn*>(&pawn4));
    moves = pawn2.calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(Board::G, Board::THREE, nullptr));
    VERIFY_EQUALS(moves.size(), 1lu);
    moves = pawn3.calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(Board::C, Board::SIX, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::D, Board::SIX, &pawn4));
    VERIFY_EQUALS(moves.size(), 2lu);
  }
  TEST_END
}

// Checks if Knight::calculatePossibleMoves returns proper moves
TEST_PROCEDURE(test2) {
  TEST_START
  {
    Board board;
    Knight knight(board, std::make_pair(Board::E, Board::FOUR), Figure::WHITE);
    auto moves = knight.calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(Board::D, Board::SIX, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::F, Board::SIX, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::G, Board::FIVE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::G, Board::THREE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::F, Board::TWO, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::D, Board::TWO, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::C, Board::THREE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::C, Board::FIVE, nullptr));
    VERIFY_EQUALS(moves.size(), 8lu);
  }
  {
    Board board;
    Knight knight(board, std::make_pair(Board::H, Board::EIGHT), Figure::BLACK);
    auto moves = knight.calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(Board::F, Board::SEVEN, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::G, Board::SIX, nullptr));
    VERIFY_EQUALS(moves.size(), 2lu);
  }
  {
    Board board;
    Knight knight(board, std::make_pair(Board::B, Board::ONE), Figure::BLACK);
    Knight enemy_knight(board, std::make_pair(Board::D, Board::TWO), Figure::WHITE);
    Pawn pawn(board, std::make_pair(Board::C, Board::THREE), Figure::BLACK);
    auto moves = knight.calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(Board::A, Board::THREE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::D, Board::TWO, &enemy_knight));
    VERIFY_EQUALS(moves.size(), 2lu);
  }
  TEST_END
}

// Checks if Bishop::calculatePossibleMoves returns proper moves
TEST_PROCEDURE(test3) {
  TEST_START
  {
    Board board;
    Bishop bishop(board, std::make_pair(Board::D, Board::FOUR), Figure::BLACK);
    auto moves = bishop.calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(Board::C, Board::THREE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::B, Board::TWO, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::A, Board::ONE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::E, Board::THREE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::F, Board::TWO, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::G, Board::ONE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::C, Board::FIVE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::B, Board::SIX, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::A, Board::SEVEN, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::E, Board::FIVE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::F, Board::SIX, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::G, Board::SEVEN, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::H, Board::EIGHT, nullptr));
    VERIFY_EQUALS(moves.size(), 13lu);
  }
  {
    Board board;
    Bishop bishop(board, std::make_pair(Board::F, Board::TWO), Figure::WHITE);
    Knight knight(board, std::make_pair(Board::D, Board::FOUR), Figure::WHITE);
    Pawn enemy_pawn(board, std::make_pair(Board::G, Board::THREE), Figure::BLACK);
    auto moves = bishop.calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(Board::E, Board::ONE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::E, Board::THREE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::G, Board::ONE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::G, Board::THREE, &enemy_pawn));
    VERIFY_EQUALS(moves.size(), 4lu);
  }
  TEST_END
}

// Checks if Rook::calculatePossibleMoves returns proper moves
TEST_PROCEDURE(test4) {
  TEST_START
  {
    Board board;
    Rook rook(board, std::make_pair(Board::E, Board::FIVE), Figure::WHITE);
    auto moves = rook.calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(Board::D, Board::FIVE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::C, Board::FIVE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::B, Board::FIVE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::A, Board::FIVE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::E, Board::SIX, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::E, Board::SEVEN, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::E, Board::EIGHT, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::F, Board::FIVE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::G, Board::FIVE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::H, Board::FIVE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::E, Board::FOUR, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::E, Board::THREE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::E, Board::TWO, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::E, Board::ONE, nullptr));
    VERIFY_EQUALS(moves.size(), 14lu);
  }
  {
    Board board;
    Rook rook(board, std::make_pair(Board::D, Board::ONE), Figure::BLACK);
    Bishop bishop(board, std::make_pair(Board::D, Board::THREE), Figure::BLACK);
    Bishop enemy_bishop(board, std::make_pair(Board::E, Board::ONE), Figure::WHITE);
    Knight enemy_knight(board, std::make_pair(Board::A, Board::ONE), Figure::WHITE);
    auto moves = rook.calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(Board::C, Board::ONE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::B, Board::ONE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::D, Board::TWO, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::A, Board::ONE, &enemy_knight));
    VERIFY_CONTAINS(moves, createMove(Board::E, Board::ONE, &enemy_bishop));
    VERIFY_EQUALS(moves.size(), 5lu);
  }
  TEST_END
}

// Checks if Queen::calculatePossibleMoves returns proper moves
TEST_PROCEDURE(test5) {
  TEST_START
  {
    Board board;
    Queen queen(board, std::make_pair(Board::E, Board::FIVE), Figure::BLACK);
    auto moves = queen.calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(Board::D, Board::SIX, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::C, Board::SEVEN, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::B, Board::EIGHT, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::E, Board::SIX, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::E, Board::SEVEN, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::E, Board::EIGHT, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::F, Board::SIX, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::G, Board::SEVEN, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::F, Board::FIVE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::G, Board::FIVE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::H, Board::FIVE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::F, Board::FOUR, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::G, Board::THREE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::H, Board::TWO, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::E, Board::FOUR, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::E, Board::THREE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::E, Board::TWO, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::E, Board::ONE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::D, Board::FOUR, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::C, Board::THREE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::B, Board::TWO, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::A, Board::ONE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::D, Board::FIVE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::C, Board::FIVE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::B, Board::FIVE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::A, Board::FIVE, nullptr));
    VERIFY_EQUALS(moves.size(), 27lu);
  }
  {
    Board board;
    Queen queen(board, std::make_pair(Board::H, Board::THREE), Figure::WHITE);
    Rook rook(board, std::make_pair(Board::G, Board::THREE), Figure::WHITE);
    Knight knight(board, std::make_pair(Board::H, Board::SIX), Figure::WHITE);
    Bishop enemy_bishop(board, std::make_pair(Board::F, Board::FIVE), Figure::BLACK);
    Pawn enemy_pawn(board, std::make_pair(Board::H, Board::TWO), Figure::BLACK);
    auto moves = queen.calculatePossibleMoves();
    VERIFY_CONTAINS(moves, createMove(Board::G, Board::FOUR, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::F, Board::FIVE, &enemy_bishop));
    VERIFY_CONTAINS(moves, createMove(Board::H, Board::FOUR, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::H, Board::FIVE, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::H, Board::TWO, &enemy_pawn));
    VERIFY_CONTAINS(moves, createMove(Board::G, Board::TWO, nullptr));
    VERIFY_CONTAINS(moves, createMove(Board::F, Board::ONE, nullptr));
    VERIFY_EQUALS(moves.size(), 7lu);
  }
  TEST_END
}

// Checks if King::calculatePossibleMoves returns proper moves
TEST_PROCEDURE(test6) {
  TEST_START
  VERIFY(false);
  TEST_END
}

TEST_PROCEDURE(test7) {
  TEST_START
  VERIFY(false);
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
    TEST("", test7);
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
