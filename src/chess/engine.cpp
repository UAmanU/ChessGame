#include "chess/engine.h"
#include "chess/opening_book.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <unordered_map>

namespace {
    constexpr int MATE_SCORE = 100000;
    constexpr int MAX_PLY = 128;
    constexpr int MAX_QUIESCENCE_PLY = 14;

    enum class Bound : unsigned char { Exact, Lower, Upper };

    struct TableEntry {
        int depth{};
        int score{};
        Bound bound{Bound::Exact};
        Move best_move{};
        bool has_best_move{false};
    };

    int piece_value(PieceType type) {
        switch (type) {
            case PieceType::Pawn: return 100;
            case PieceType::Knight: return 320;
            case PieceType::Bishop: return 335;
            case PieceType::Rook: return 500;
            case PieceType::Queen: return 900;
            case PieceType::King: return 20000;
            default: return 0;
        }
    }

    int mate_score_to_table(int score, int ply) {
        if (score > MATE_SCORE - MAX_PLY) return score + ply;
        if (score < -MATE_SCORE + MAX_PLY) return score - ply;
        return score;
    }

    int mate_score_from_table(int score, int ply) {
        if (score > MATE_SCORE - MAX_PLY) return score - ply;
        if (score < -MATE_SCORE + MAX_PLY) return score + ply;
        return score;
    }

    int move_index(const Move& move) {
        return (move.from_row * 8 + move.from_col) * 64 + move.to_row * 8 + move.to_col;
    }

    bool is_capture(const Board& board, const Move& move) {
        if (!board.at(move.to_row, move.to_col).is_empty()) return true;
        const Piece attacker = board.at(move.from_row, move.from_col);
        return attacker.type == PieceType::Pawn && move.from_col != move.to_col;
    }

    int capture_priority(const Board& board, const Move& move) {
        Piece victim = board.at(move.to_row, move.to_col);
        if (victim.is_empty()) victim.type = PieceType::Pawn; // En passant.
        const Piece attacker = board.at(move.from_row, move.from_col);
        return 100000 + piece_value(victim.type) * 16 - piece_value(attacker.type);
    }
}

struct ChessSearchContext {
    std::unordered_map<std::uint64_t, TableEntry> transpositions;
    std::array<std::array<std::optional<Move>, 2>, MAX_PLY> killer_moves{};
    std::array<std::array<int, 4096>, 2> history_scores{};
    std::uint64_t nodes{0};

    ChessSearchContext() { transpositions.reserve(1U << 18U); }

    void order_moves(const Board& board, std::vector<Move>& moves, int ply, const std::optional<Move>& table_move) const {
        const int side = board.side_to_move() == ColorSide::White ? 0 : 1;
        std::stable_sort(moves.begin(), moves.end(), [&](const Move& left, const Move& right) {
            auto score = [&](const Move& move) {
                if (table_move && move == *table_move) return 2000000;
                const Piece attacker = board.at(move.from_row, move.from_col);
                if (is_capture(board, move)) return capture_priority(board, move);
                if (attacker.type == PieceType::Pawn && (move.to_row == 0 || move.to_row == 7)) return 90000;
                if (ply < MAX_PLY) {
                    if (killer_moves[ply][0] && move == *killer_moves[ply][0]) return 80000;
                    if (killer_moves[ply][1] && move == *killer_moves[ply][1]) return 79000;
                }
                return history_scores[side][move_index(move)];
            };
            return score(left) > score(right);
        });
    }

    void record_quiet_cutoff(ColorSide side, const Move& move, int depth, int ply) {
        if (ply < MAX_PLY) {
            if (!killer_moves[ply][0] || *killer_moves[ply][0] != move) {
                killer_moves[ply][1] = killer_moves[ply][0];
                killer_moves[ply][0] = move;
            }
        }
        int& score = history_scores[side == ColorSide::White ? 0 : 1][move_index(move)];
        score = std::min(100000, score + depth * depth);
    }
};

int ChessEngine::evaluate(const Board& board) {
    int score = 0;
    int phase_material = 0;
    int bishop_count[2]{};
    int pawns_on_file[2][8]{};
    int non_pawn_pieces = 0;

    for (int row = 0; row < 8; ++row) for (int col = 0; col < 8; ++col) {
        const Piece piece = board.at(row, col);
        if (piece.is_empty()) continue;
        const int side = piece.side == ColorSide::White ? 0 : 1;
        if (piece.type == PieceType::Pawn) ++pawns_on_file[side][col];
        if (piece.type == PieceType::Bishop) ++bishop_count[side];
        if (piece.type == PieceType::Knight) phase_material += 1;
        if (piece.type == PieceType::Bishop) phase_material += 1;
        if (piece.type == PieceType::Rook) phase_material += 2;
        if (piece.type == PieceType::Queen) phase_material += 4;
        if (piece.type != PieceType::Pawn && piece.type != PieceType::King) ++non_pawn_pieces;
    }
    phase_material = std::min(24, phase_material);
    const bool opening_phase = non_pawn_pieces >= 10;

    for (int row = 0; row < 8; ++row) for (int col = 0; col < 8; ++col) {
        const Piece piece = board.at(row, col);
        if (piece.is_empty()) continue;
        const int side = piece.side == ColorSide::White ? 0 : 1;
        const int sign = side == 0 ? 1 : -1;
        const int home_row = side == 0 ? 7 : 0;
        const int progress = side == 0 ? 6 - row : row - 1;
        int positional = 0;

        if (piece.type == PieceType::Pawn) {
            positional += progress * 2;
            if (col == 3 || col == 4) positional += 8;
            if ((row == 3 || row == 4) && (col == 3 || col == 4)) positional += 18;

            if (pawns_on_file[side][col] > 1) positional -= 12 * (pawns_on_file[side][col] - 1);
            const bool isolated = (col == 0 || pawns_on_file[side][col - 1] == 0) &&
                                  (col == 7 || pawns_on_file[side][col + 1] == 0);
            if (isolated) positional -= 10;

            bool passed = true;
            for (int enemy_col = std::max(0, col - 1); enemy_col <= std::min(7, col + 1); ++enemy_col) {
                for (int enemy_row = 0; enemy_row < 8; ++enemy_row) {
                    const Piece other = board.at(enemy_row, enemy_col);
                    if (other.type != PieceType::Pawn || other.side == piece.side) continue;
                    if ((side == 0 && enemy_row < row) || (side == 1 && enemy_row > row)) passed = false;
                }
            }
            if (passed) positional += progress * progress * 2;
            if ((col == 0 || col == 7) && progress > 0 && opening_phase) positional -= progress * 8;
        } else if (piece.type == PieceType::Knight) {
            const int center_distance = std::abs(3 - row) + std::abs(3 - col);
            positional += std::max(0, 16 - center_distance * 3);
            if (opening_phase && row == home_row && (col == 1 || col == 6)) positional -= 22;
        } else if (piece.type == PieceType::Bishop) {
            const int center_distance = std::abs(3 - row) + std::abs(3 - col);
            positional += std::max(0, 8 - center_distance);
            if (opening_phase && row == home_row && (col == 2 || col == 5)) positional -= 20;
            if (col == 0 || col == 7) positional -= 8;
        } else if (piece.type == PieceType::Rook) {
            if (pawns_on_file[0][col] == 0 && pawns_on_file[1][col] == 0) positional += 18;
            else if (pawns_on_file[side][col] == 0) positional += 9;
            if ((side == 0 && row == 1) || (side == 1 && row == 6)) positional += 14;
        } else if (piece.type == PieceType::Queen && opening_phase) {
            if (row != home_row || col != 3) positional -= 10;
        } else if (piece.type == PieceType::King) {
            const int endgame_bonus = (24 - phase_material) * 2;
            const int king_center_distance = std::abs(3 - row) + std::abs(3 - col);
            positional += endgame_bonus * std::max(0, 6 - king_center_distance) / 6;
            if (opening_phase && row == home_row && (col == 2 || col == 6)) positional += 35;
            else if (opening_phase && row == home_row && col == 4) positional -= 14;
        }

        score += sign * (piece_value(piece.type) + positional);
    }

    if (bishop_count[0] >= 2) score += 28;
    if (bishop_count[1] >= 2) score -= 28;
    return score;
}

std::optional<Move> ChessEngine::find_best_move(const Board& board, const std::vector<Move>& move_history) const {
    std::vector<Move> root_moves = board.legal_moves();
    if (root_moves.empty()) return std::nullopt;
    if (const auto book_move = OpeningBook::find_move(move_history, root_moves)) return book_move;

    ChessSearchContext context;
    std::optional<Move> best_move;
    std::optional<Move> previous_best;
    const bool maximizing = board.side_to_move() == ColorSide::White;

    // Iterative deepening preserves a usable move at every completed depth and feeds ordering at the next.
    for (int depth = 1; depth <= depth_; ++depth) {
        context.order_moves(board, root_moves, 0, previous_best);
        int alpha = std::numeric_limits<int>::min() + 1;
        int beta = std::numeric_limits<int>::max();
        int iteration_best = maximizing ? alpha : beta;
        std::optional<Move> iteration_move;

        for (const Move& move : root_moves) {
            Board child = board;
            (void)child.apply_unchecked(move); // Moves came from the legal root list above.
            const int score = search(child, depth - 1, alpha, beta, 1, context);
            if ((maximizing && score > iteration_best) || (!maximizing && score < iteration_best)) {
                iteration_best = score;
                iteration_move = move;
            }
            if (maximizing) alpha = std::max(alpha, iteration_best);
            else beta = std::min(beta, iteration_best);
            if (alpha >= beta) break;
        }
        if (iteration_move) {
            best_move = iteration_move;
            previous_best = iteration_move;
        }
    }
    return best_move;
}

int ChessEngine::search(Board& board, int depth, int alpha, int beta, int ply, ChessSearchContext& context) const {
    ++context.nodes;
    if (depth <= 0) return quiescence(board, alpha, beta, ply, 0, context);

    const int original_alpha = alpha;
    const int original_beta = beta;
    const std::uint64_t key = board.position_key();
    std::optional<Move> table_move;
    if (const auto entry = context.transpositions.find(key); entry != context.transpositions.end()) {
        table_move = entry->second.has_best_move ? std::optional<Move>(entry->second.best_move) : std::nullopt;
        if (entry->second.depth >= depth) {
            const int cached_score = mate_score_from_table(entry->second.score, ply);
            if (entry->second.bound == Bound::Exact) return cached_score;
            if (entry->second.bound == Bound::Lower) alpha = std::max(alpha, cached_score);
            else beta = std::min(beta, cached_score);
            if (alpha >= beta) return cached_score;
        }
    }

    std::vector<Move> moves = board.legal_moves();
    if (moves.empty()) {
        if (!board.in_check(board.side_to_move())) return 0;
        return board.side_to_move() == ColorSide::White ? -MATE_SCORE + ply : MATE_SCORE - ply;
    }

    context.order_moves(board, moves, ply, table_move);
    const bool maximizing = board.side_to_move() == ColorSide::White;
    int best_score = maximizing ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();
    std::optional<Move> best_move;

    for (const Move& move : moves) {
        Board child = board;
        (void)child.apply_unchecked(move); // Avoid regenerating legal moves for a verified move.
        const int score = search(child, depth - 1, alpha, beta, ply + 1, context);
        if ((maximizing && score > best_score) || (!maximizing && score < best_score)) {
            best_score = score;
            best_move = move;
        }

        if (maximizing) alpha = std::max(alpha, best_score);
        else beta = std::min(beta, best_score);
        if (beta <= alpha) {
            if (!is_capture(board, move)) context.record_quiet_cutoff(board.side_to_move(), move, depth, ply);
            break;
        }
    }

    TableEntry entry;
    entry.depth = depth;
    entry.score = mate_score_to_table(best_score, ply);
    entry.bound = best_score <= original_alpha ? Bound::Upper : (best_score >= original_beta ? Bound::Lower : Bound::Exact);
    if (best_move) {
        entry.best_move = *best_move;
        entry.has_best_move = true;
    }
    auto existing = context.transpositions.find(key);
    if (existing == context.transpositions.end() || existing->second.depth <= depth || entry.bound == Bound::Exact) {
        context.transpositions[key] = entry;
    }
    return best_score;
}

int ChessEngine::quiescence(Board& board, int alpha, int beta, int ply, int qply, ChessSearchContext& context) const {
    ++context.nodes;
    const bool in_check = board.in_check(board.side_to_move());
    std::vector<Move> moves = board.legal_moves();
    if (moves.empty()) {
        if (!in_check) return 0;
        return board.side_to_move() == ColorSide::White ? -MATE_SCORE + ply : MATE_SCORE - ply;
    }

    const bool maximizing = board.side_to_move() == ColorSide::White;
    int best_score;
    if (!in_check) {
        best_score = evaluate(board);
        if (maximizing) {
            if (best_score >= beta) return best_score;
            alpha = std::max(alpha, best_score);
        } else {
            if (best_score <= alpha) return best_score;
            beta = std::min(beta, best_score);
        }
        if (qply >= MAX_QUIESCENCE_PLY) return best_score;
        std::erase_if(moves, [&](const Move& move) {
            const Piece attacker = board.at(move.from_row, move.from_col);
            const bool promotion = attacker.type == PieceType::Pawn && (move.to_row == 0 || move.to_row == 7);
            return !is_capture(board, move) && !promotion;
        });
        if (moves.empty()) return best_score;
    } else if (qply >= MAX_QUIESCENCE_PLY) {
        return evaluate(board);
    } else {
        best_score = maximizing ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();
    }

    context.order_moves(board, moves, std::min(ply, MAX_PLY - 1), std::nullopt);
    for (const Move& move : moves) {
        Board child = board;
        (void)child.apply_unchecked(move); // Captures were selected from the current legal list.
        const int score = quiescence(child, alpha, beta, ply + 1, qply + 1, context);
        if (maximizing) {
            best_score = std::max(best_score, score);
            alpha = std::max(alpha, best_score);
        } else {
            best_score = std::min(best_score, score);
            beta = std::min(beta, best_score);
        }
        if (beta <= alpha) break;
    }
    return best_score;
}
