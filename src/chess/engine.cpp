#include "chess/engine.h"
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
    int score=0;
    for(int r=0;r<8;++r) for(int c=0;c<8;++c) { Piece p=board.at(r,c); int position= (p.type==PieceType::Pawn ? (p.side==ColorSide::White ? 6-r : r-1)*3 : (3-std::abs(3-c))*2); int v=value(p.type)+position; score += p.is_empty()?0:(p.side==ColorSide::White?v:-v); }
    return score;
}
std::optional<Move> ChessEngine::find_best_move(const Board& board) const {
    auto moves=board.legal_moves(); if(moves.empty()) return std::nullopt;
    order_moves(board, moves);
    bool maximizing=board.side_to_move()==ColorSide::White; int best=maximizing?std::numeric_limits<int>::min():std::numeric_limits<int>::max(); std::optional<Move> best_move;
    for(const auto& move:moves) { Board next=board; (void)next.apply_move(move); int score=search(next,depth_-1,std::numeric_limits<int>::min()+1,std::numeric_limits<int>::max(),1); if((maximizing&&score>best)||(!maximizing&&score<best)){best=score;best_move=move;} }
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
