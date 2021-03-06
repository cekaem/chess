#ifndef BOARD_H
#define BOARD_H

#include <array>
#include <cassert>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include "Field.h"
#include "Figure.h"


#ifdef _ASSERTS_ON_
#define BoardAssert(_board_, _condition_)\
  if ((_condition_) == false) {\
    std::cerr << (_board_).createFEN() << std::endl;\
    assert(_condition_);\
  }
#else
#define NOOP
#define BoardAssert(_board_, _condition_) NOOP
#endif  // _DEBUG_
 

class BoardDrawer;

class Board {
 public:
  static int number_of_copies_;
  constexpr static size_t BoardSize = 8;

  enum class GameStatus {
    NONE,
    WHITE_WON,
    BLACK_WON, 
    DRAW
  };

  struct NoFigureException : std::exception {
    NoFigureException(const Field& field) : field_(field) {}
    const Field field_;
  };

  struct FieldNotEmptyException : std::exception {
    FieldNotEmptyException(const Field& field, const Figure* figure)
      : field_(field), figure_(figure) {}
    const Field field_;
    const Figure* figure_;
  };

  struct IllegalMoveException : std::exception {
    IllegalMoveException(const Figure* figure, Field field)
      : figure_(figure), field_(field) {}
    const Figure* figure_;
    Field field_;
  };

  struct BadBoardStatusException : std::exception {
    BadBoardStatusException(const Board* board) : board_(board) {}
    const Board* board_;
  };

  struct ReversibleMove {
    ReversibleMove(
        Figure::Move& move,
        std::unique_ptr<Figure> bf,
        std::unique_ptr<Figure> pp,
        Field::Letter epf,
        std::array<bool, static_cast<int>(Figure::Move::Castling::LAST)>& cast,
        unsigned hc,
        unsigned fm,
        Figure::Color stm)
      : old_field(move.old_field),
        new_field(move.new_field),
        bitten_figure(std::move(bf)),
        en_passant_file(epf),
        promoted_pawn(std::move(pp)),
        castling_move(move.castling != Figure::Move::Castling::LAST),
        castlings(cast),
        halfmove_clock(hc),
        fullmove_number(fm),
        side_to_move(stm) {
    }

    ReversibleMove(ReversibleMove&& other)
      : old_field(other.old_field),
        new_field(other.new_field),
        bitten_figure(std::move(other.bitten_figure)),
        en_passant_file(other.en_passant_file),
        promoted_pawn(std::move(other.promoted_pawn)),
        castling_move(other.castling_move),
        castlings(other.castlings),
        halfmove_clock(other.halfmove_clock),
        fullmove_number(other.fullmove_number),
        side_to_move(other.side_to_move) {
    }

    Field old_field;
    Field new_field;
    std::unique_ptr<Figure> bitten_figure;
    Field::Letter en_passant_file{Field::Letter::NONE};
    std::unique_ptr<Figure> promoted_pawn;
    bool castling_move{false};
    std::array<bool, static_cast<int>(Figure::Move::Castling::LAST)> castlings{true, true, true, true};
    unsigned halfmove_clock{0};
    unsigned fullmove_number{1};
    Figure::Color side_to_move{Figure::WHITE};
  };

  class ReversibleMoveWrapper {
   public:
    ReversibleMoveWrapper(Board& board) : board_(board) {
    }

    ~ReversibleMoveWrapper() {
      board_.undoLastReversibleMove();
    }

   private:
    Board& board_;
  };

  Board() noexcept;
  Board(const Board& other) noexcept;

  bool isMoveValid(Field old_field, Field new_field);
  bool addFigure(const char fen_char, Field field);
  const Figure* addFigure(Figure::Type type, Field field, Figure::Color color);
  std::unique_ptr<Figure> removeFigure(Field field);
  GameStatus makeMove(Field old_field, Field new_field, Figure::Type promotion = Figure::PAWN, bool rev_mode = false);
  GameStatus makeMove(Figure::Move move, bool rev_mode = false);
  ReversibleMoveWrapper makeReversibleMove(Figure::Move move);
  const Figure* getFigure(Field field) const noexcept;
  const std::vector<std::unique_ptr<Figure>>& getFigures() const noexcept { return figures_; }
  std::vector<const Figure*> getFigures(Figure::Color color) const noexcept;
  const auto& getFields() const noexcept { return fields_; }
  Field::Letter getEnPassantFile() const noexcept { return en_passant_file_; }
  std::string getFENForCastlings() const noexcept;
  unsigned getHalfMoveClock() const noexcept { return halfmove_clock_ / 2; }
  unsigned getFullMoveNumber() const noexcept { return fullmove_number_; }
  const King* getKing(Figure::Color color) const noexcept;
  void clearBoard();
  void setStandardBoard();
  Figure::Color getSideToMove() const { return side_to_move_; }
  void setSideToMove(Figure::Color side_to_move) { side_to_move_ = side_to_move; }
  GameStatus getGameStatus(Figure::Color color);
  void addBoardDrawer(BoardDrawer* drawer) noexcept;
  void removeBoardDrawer(BoardDrawer* drawer) noexcept;
  std::vector<Figure::Move> calculateMovesForFigure(const Figure* figure);
  std::vector<Figure::Move> calculateMovesForFigures(Figure::Color color);
  bool isKingChecked(Figure::Color color);
  bool isKingCheckmated(Figure::Color color);
  bool isKingStalemated(Figure::Color color);
  bool canKingCastle(Figure::Color color) const;

  std::string createFEN() const;
  bool setBoardFromFEN(const std::string& fen);

  bool operator==(const Board& other) const noexcept;
  bool operator!=(const Board& other) const noexcept;

  friend std::ostream& operator<<(std::ostream& ostr, const Board& board);

  void undoLastReversibleMove();
  void undoAllReversibleMoves();

 private:
  Board& operator=(const Board& other) = delete;
  Board(Board&& other) = delete;

  GameStatus isCheckMate();
  bool isDraw() const;
  void onGameFinished(GameStatus status) noexcept;
  bool isMoveValid(Figure::Move& move, Figure::Color color);
  bool isEnPassantCapture(const Figure::Move& move) const;
  bool canCastle(Figure::Move::Castling castling) const;
  void moveFigure(Field old_field, Field new_field);
  void updateCastlings(const Figure::Move& move);
  bool addFiguresForOneLineFromFen(const std::string& fen, size_t line);
  bool setCastlingsFromFen(const std::string& fen);
  bool setEnPassantFileFromFen(const std::string& fen, bool white_to_move);

  Field::Letter en_passant_file_{Field::Letter::NONE};
  std::vector<std::unique_ptr<Figure>> figures_;
  std::vector<BoardDrawer*> drawers_;
  std::array<std::array<Figure*, BoardSize>, BoardSize> fields_;
  std::vector<ReversibleMove> reversible_moves_;
  std::array<bool, static_cast<int>(Figure::Move::Castling::LAST)> castlings_{true, true, true, true};
  unsigned halfmove_clock_{0};
  unsigned fullmove_number_{1};
  Figure::Color side_to_move_{Figure::WHITE};
};

class BoardDrawer {
 public:
  virtual void onFigureAdded(Figure::Type type, Figure::Color color, Field field) = 0;
  virtual void onFigureRemoved(Field field) = 0;
  virtual void onFigureMoved(Figure::Move move) = 0;
  virtual void onGameFinished(Board::GameStatus status) = 0;
};

std::ostream& operator<<(std::ostream& ostr, Board::GameStatus status);

#endif  // BOARD_H
