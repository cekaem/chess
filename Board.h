#ifndef BOARD_H
#define BOARD_H

#include <array>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include "Field.h"
#include "Figure.h"

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
    ReversibleMove(Figure::Move& move, std::unique_ptr<Figure> bf, std::unique_ptr<Figure> pp, const Pawn* epp, bool moved)
      : old_field(move.old_field),
        new_field(move.new_field),
        bitten_figure(std::move(bf)),
        en_passant_pawn(epp),
        promoted_pawn(std::move(pp)),
        castling_move(move.castling != Figure::Move::Castling::NONE),
        moved_at_least_once(moved) {
    }

    ReversibleMove(ReversibleMove&& other)
      : old_field(other.old_field),
        new_field(other.new_field),
        bitten_figure(std::move(other.bitten_figure)),
        en_passant_pawn(other.en_passant_pawn),
        promoted_pawn(std::move(other.promoted_pawn)),
        castling_move(other.castling_move),
        moved_at_least_once(other.moved_at_least_once) {
    }

    Field old_field;
    Field new_field;
    std::unique_ptr<Figure> bitten_figure;
    const Pawn* en_passant_pawn{nullptr};
    std::unique_ptr<Figure> promoted_pawn;
    bool castling_move{false};
    bool moved_at_least_once{false};
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

  const Figure* addFigure(Figure::Type type, Field field, Figure::Color color);
  std::unique_ptr<Figure> removeFigure(Field field);
  GameStatus makeMove(Field old_field, Field new_field, Figure::Type promotion = Figure::PAWN, bool rev_mode = false);
  GameStatus makeMove(Figure::Move move, bool rev_mode = false);
  ReversibleMoveWrapper makeReversibleMove(Figure::Move move);
  const Figure* getFigure(Field field) const noexcept;
  const std::vector<std::unique_ptr<Figure>>& getFigures() const noexcept { return figures_; }
  std::vector<const Figure*> getFigures(Figure::Color color) const noexcept;
  const auto& getFields() const noexcept { return fields_; }
  const Pawn* getEnPassantPawn() const noexcept { return en_passant_pawn_; }
  const King* getKing(Figure::Color color) const noexcept;
  void setStandardBoard();
  GameStatus getGameStatus(Figure::Color color);
  void addBoardDrawer(BoardDrawer* drawer) noexcept;
  void removeBoardDrawer(BoardDrawer* drawer) noexcept;
  std::vector<Figure::Move> calculateMovesForFigure(const Figure* figure);
  bool isKingChecked(Figure::Color color);
  bool isKingCheckmated(Figure::Color color);
  bool isKingStalemated(Figure::Color color);

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
  void handleCastling(Figure::Move& move);
  bool isMoveValid(Figure::Move& move, Figure::Color color);
  void moveFigure(Field old_field, Field new_field, bool moved);

  const Pawn* en_passant_pawn_{nullptr};
  std::vector<std::unique_ptr<Figure>> figures_;
  std::vector<BoardDrawer*> drawers_;
  std::array<std::array<Figure*, BoardSize>, BoardSize> fields_;
  std::vector<ReversibleMove> reversible_moves_;
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
