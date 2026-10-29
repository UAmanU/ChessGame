#pragma once
#include "chess/board.h"
#include <optional>

class ChessEngine {
public:
    explicit ChessEngine(int depth = 3) : depth_(depth) {}
    [[nodiscard]] std::optional<Move> find_best_move(const Board& board) const;
private:
    int depth_;
    // White maximizes the material score and Black minimizes it; alpha-beta prunes branches.
    [[nodiscard]] int search(Board& board, int depth, int alpha, int beta, int ply) const;
    [[nodiscard]] static int evaluate(const Board& board);
};
