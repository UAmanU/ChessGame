#pragma once
#include "chess/board.h"
#include <optional>
#include <vector>

struct ChessSearchContext;

class ChessEngine {
public:
    explicit ChessEngine(int depth = 5) : depth_(depth) {}
    [[nodiscard]] std::optional<Move> find_best_move(const Board& board, const std::vector<Move>& move_history = {}) const;
private:
    int depth_;
    // White maximizes the material score and Black minimizes it.
    [[nodiscard]] int search(Board& board, int depth, int alpha, int beta, int ply, ChessSearchContext& context) const;
    [[nodiscard]] int quiescence(Board& board, int alpha, int beta, int ply, int qply, ChessSearchContext& context) const;
    [[nodiscard]] static int evaluate(const Board& board);
};
