#pragma once
#include <array>
#include <cstdint>
#include <vector>
#include "chess/move.h"

class Board {
public:
    Board();
    [[nodiscard]] Piece at(int row, int col) const;
    [[nodiscard]] ColorSide side_to_move() const noexcept { return side_to_move_; }
    [[nodiscard]] std::uint64_t position_key() const noexcept;
    [[nodiscard]] std::vector<Move> legal_moves() const;
    [[nodiscard]] std::vector<Move> legal_moves_from(int row, int col) const;
    [[nodiscard]] bool is_checkmate() const;
    [[nodiscard]] bool is_stalemate() const;
    [[nodiscard]] bool in_check(ColorSide side) const;
    [[nodiscard]] bool apply_move(const Move& move);
    void reset();
private:
    friend class ChessEngine;
    // Generate candidates first, then reject moves that leave the moving king in check.
    std::array<Piece, 64> squares_{};
    ColorSide side_to_move_{ColorSide::White};
    bool white_king_moved_{false}, black_king_moved_{false};
    bool white_rook_a_moved_{false}, white_rook_h_moved_{false};
    bool black_rook_a_moved_{false}, black_rook_h_moved_{false};
    int en_passant_square_{-1};
    [[nodiscard]] static bool inside(int row, int col);
    [[nodiscard]] bool attacked(int row, int col, ColorSide by_side) const;
    [[nodiscard]] std::vector<Move> pseudo_moves() const;
    [[nodiscard]] bool apply_unchecked(const Move& move);
};
