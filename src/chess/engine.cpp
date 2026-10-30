#include "chess/engine.h"
#include "chess/opening_book.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace {
    constexpr int MATE_SCORE=100000;
    int value(PieceType type) { switch(type) { case PieceType::Pawn:return 100; case PieceType::Knight:return 320; case PieceType::Bishop:return 330; case PieceType::Rook:return 500; case PieceType::Queen:return 900; case PieceType::King:return 20000; default:return 0; } }

    // Good captures and promotions first make alpha-beta cut off much earlier.
    int move_priority(const Board& board, const Move& move) {
        const Piece attacker = board.at(move.from_row, move.from_col);
        const Piece target = board.at(move.to_row, move.to_col);
        int score = 0;
        if (!target.is_empty()) score += 10 * value(target.type) - value(attacker.type);
        else if (attacker.type == PieceType::Pawn && move.from_col != move.to_col) score += 10 * value(PieceType::Pawn) - value(attacker.type);
        if (attacker.type == PieceType::Pawn && (move.to_row == 0 || move.to_row == 7)) score += value(PieceType::Queen);
        return score;
    }

    void order_moves(const Board& board, std::vector<Move>& moves) {
        std::stable_sort(moves.begin(), moves.end(), [&](const Move& left, const Move& right) {
            return move_priority(board, left) > move_priority(board, right);
        });
    }
}
int ChessEngine::evaluate(const Board& board) {
    int score = 0;
    int undeveloped_phase_pieces = 0;
    for (int row = 0; row < 8; ++row) for (int col = 0; col < 8; ++col) {
        const Piece piece = board.at(row, col);
        if (!piece.is_empty() && piece.type != PieceType::Pawn && piece.type != PieceType::King) ++undeveloped_phase_pieces;
    }
    const bool opening_phase = undeveloped_phase_pieces >= 10;

    for (int row = 0; row < 8; ++row) for (int col = 0; col < 8; ++col) {
        const Piece piece = board.at(row, col);
        if (piece.is_empty()) continue;

        const int sign = piece.side == ColorSide::White ? 1 : -1;
        const int home_row = piece.side == ColorSide::White ? 7 : 0;
        const int pawn_progress = piece.side == ColorSide::White ? 6 - row : row - 1;
        int positional_score = 0;

        if (piece.type == PieceType::Pawn) {
            // Pawn progress is useful, but must not make a needless edge-pawn push look best.
            positional_score += pawn_progress * 2;
            if (col == 3 || col == 4) positional_score += 10;
            if ((row == 3 || row == 4) && (col == 3 || col == 4)) positional_score += 16;
            if ((col == 0 || col == 7) && pawn_progress > 0) positional_score -= pawn_progress * 8;
        } else if (piece.type == PieceType::Knight) {
            const int center_distance = std::abs(3 - row) + std::abs(3 - col);
            positional_score += std::max(0, 12 - center_distance * 2);
            if (opening_phase && row == home_row && (col == 1 || col == 6)) positional_score -= 18;
        } else if (piece.type == PieceType::Bishop && opening_phase && row == home_row && (col == 2 || col == 5)) {
            positional_score -= 16;
        } else if (piece.type == PieceType::King && opening_phase) {
            if (row == home_row && (col == 2 || col == 6)) positional_score += 28;
            else if (row == home_row && col == 4) positional_score -= 8;
        }

        score += sign * (value(piece.type) + positional_score);
    }
    return score;
}
std::optional<Move> ChessEngine::find_best_move(const Board& board, const std::vector<Move>& move_history) const {
    auto moves=board.legal_moves(); if(moves.empty()) return std::nullopt;
    if (const auto book_move = OpeningBook::find_move(move_history, moves)) return book_move;
    order_moves(board, moves);
    const bool maximizing=board.side_to_move()==ColorSide::White;
    int alpha=std::numeric_limits<int>::min()+1;
    int beta=std::numeric_limits<int>::max();
    int best=maximizing?alpha:beta;
    std::optional<Move> best_move;
    for(const auto& move:moves) {
        Board next=board;
        (void)next.apply_move(move);
        const int score=search(next,depth_-1,alpha,beta,1);
        if((maximizing&&score>best)||(!maximizing&&score<best)) { best=score; best_move=move; }
        if(maximizing) alpha=std::max(alpha,best);
        else beta=std::min(beta,best);
    }
    return best_move;
}
int ChessEngine::search(Board& board,int depth,int alpha,int beta,int ply) const {
    auto moves=board.legal_moves();
    if(moves.empty()) return board.in_check(board.side_to_move()) ? (board.side_to_move()==ColorSide::White ? -MATE_SCORE+ply : MATE_SCORE-ply) : 0;
    if(depth<=0) return evaluate(board);
    order_moves(board, moves);
    bool maximizing=board.side_to_move()==ColorSide::White; int best=maximizing?std::numeric_limits<int>::min():std::numeric_limits<int>::max();
    for(const auto& move:moves) { Board next=board; (void)next.apply_move(move); int score=search(next,depth-1,alpha,beta,ply+1); if(maximizing){best=std::max(best,score);alpha=std::max(alpha,best);}else{best=std::min(best,score);beta=std::min(beta,best);} if(beta<=alpha) break; }
    return best;
}
