#include "chess/board.h"
#include <algorithm>
#include <cmath>

namespace {
    ColorSide opposite(ColorSide side) { return side == ColorSide::White ? ColorSide::Black : ColorSide::White; }
}

Board::Board() { reset(); }
bool Board::inside(int row, int col) { return row >= 0 && row < 8 && col >= 0 && col < 8; }
Piece Board::at(int row, int col) const { return inside(row, col) ? squares_[row * 8 + col] : Piece{}; }

std::uint64_t Board::position_key() const noexcept {
    // FNV-1a over pieces and all move-right state that changes legal moves.
    std::uint64_t key = 14695981039346656037ULL;
    const auto add_byte = [&key](std::uint8_t value) {
        key ^= value;
        key *= 1099511628211ULL;
    };
    for (const Piece piece : squares_) {
        add_byte(static_cast<std::uint8_t>(piece.type));
        add_byte(static_cast<std::uint8_t>(piece.side));
    }
    add_byte(static_cast<std::uint8_t>(side_to_move_));
    const std::uint8_t castling_rights = static_cast<std::uint8_t>(
        (!white_king_moved_ ? 1 : 0) |
        (!black_king_moved_ ? 2 : 0) |
        (!white_rook_a_moved_ ? 4 : 0) |
        (!white_rook_h_moved_ ? 8 : 0) |
        (!black_rook_a_moved_ ? 16 : 0) |
        (!black_rook_h_moved_ ? 32 : 0));
    add_byte(castling_rights);
    add_byte(static_cast<std::uint8_t>(en_passant_square_ + 1));
    return key;
}

void Board::reset() {
    squares_.fill(Piece{});
    constexpr PieceType back[] = {PieceType::Rook, PieceType::Knight, PieceType::Bishop, PieceType::Queen, PieceType::King, PieceType::Bishop, PieceType::Knight, PieceType::Rook};
    for (int col = 0; col < 8; ++col) {
        squares_[col] = {back[col], ColorSide::Black}; squares_[8 + col] = {PieceType::Pawn, ColorSide::Black};
        squares_[48 + col] = {PieceType::Pawn, ColorSide::White}; squares_[56 + col] = {back[col], ColorSide::White};
    }
    side_to_move_ = ColorSide::White;
    white_king_moved_ = black_king_moved_ = false;
    white_rook_a_moved_ = white_rook_h_moved_ = black_rook_a_moved_ = black_rook_h_moved_ = false;
    en_passant_square_ = -1;
}

bool Board::attacked(int row, int col, ColorSide by_side) const {
    const int pawn_source_row = row + (by_side == ColorSide::White ? 1 : -1);
    for (int dc : {-1, 1}) if (inside(pawn_source_row, col + dc)) {
        Piece p = at(pawn_source_row, col + dc); if (p.side == by_side && p.type == PieceType::Pawn) return true;
    }
    constexpr int knight_steps[8][2] = {{-2,-1},{-2,1},{-1,-2},{-1,2},{1,-2},{1,2},{2,-1},{2,1}};
    for (auto& step : knight_steps) { Piece p = at(row + step[0], col + step[1]); if (inside(row + step[0], col + step[1]) && p.side == by_side && p.type == PieceType::Knight) return true; }
    for (int dr = -1; dr <= 1; ++dr) for (int dc = -1; dc <= 1; ++dc) if (dr || dc) {
        int r = row + dr, c = col + dc, distance = 1;
        while (inside(r, c)) {
            Piece p = at(r, c);
            if (!p.is_empty()) {
                if (p.side == by_side && ((distance == 1 && p.type == PieceType::King) || p.type == PieceType::Queen || (dr == 0 || dc == 0 ? p.type == PieceType::Rook : p.type == PieceType::Bishop))) return true;
                break;
            }
            r += dr; c += dc; ++distance;
        }
    }
    return false;
}

bool Board::in_check(ColorSide side) const {
    for (int i = 0; i < 64; ++i) if (squares_[i].type == PieceType::King && squares_[i].side == side) return attacked(i / 8, i % 8, opposite(side));
    return true;
}

std::vector<Move> Board::pseudo_moves() const {
    std::vector<Move> moves;
    auto add = [&](int fr, int fc, int tr, int tc) { moves.push_back({fr, fc, tr, tc, PieceType::Queen}); };
    for (int row = 0; row < 8; ++row) for (int col = 0; col < 8; ++col) {
        Piece p = at(row, col); if (p.is_empty() || p.side != side_to_move_) continue;
        if (p.type == PieceType::Pawn) {
            int dir = p.side == ColorSide::White ? -1 : 1, start = p.side == ColorSide::White ? 6 : 1;
            if (inside(row + dir, col) && at(row + dir, col).is_empty()) {
                add(row, col, row + dir, col);
                if (row == start && at(row + 2 * dir, col).is_empty()) add(row, col, row + 2 * dir, col);
            }
            for (int dc : {-1, 1}) if (inside(row + dir, col + dc)) {
                Piece target = at(row + dir, col + dc);
                if ((!target.is_empty() && target.side != p.side) || (en_passant_square_ == (row + dir) * 8 + col + dc)) add(row, col, row + dir, col + dc);
            }
        } else if (p.type == PieceType::Knight) {
            constexpr int steps[8][2] = {{-2,-1},{-2,1},{-1,-2},{-1,2},{1,-2},{1,2},{2,-1},{2,1}};
            for (auto& s : steps) { int r=row+s[0], c=col+s[1]; if (inside(r,c) && (at(r,c).is_empty() || at(r,c).side != p.side)) add(row,col,r,c); }
        } else {
            for (int dr=-1; dr<=1; ++dr) for (int dc=-1; dc<=1; ++dc) {
                if ((!dr && !dc) || (p.type == PieceType::Bishop && (dr == 0 || dc == 0)) || (p.type == PieceType::Rook && dr != 0 && dc != 0)) continue;
                for (int n=1; n<8; ++n) { int r=row+dr*n, c=col+dc*n; if (!inside(r,c)) break; Piece t=at(r,c); if (t.is_empty()) add(row,col,r,c); else { if (t.side != p.side) add(row,col,r,c); break; } if (p.type == PieceType::King) break; }
            }
            if (p.type == PieceType::King && !in_check(p.side)) {
                bool white = p.side == ColorSide::White; int home = white ? 7 : 0;
                bool king_moved = white ? white_king_moved_ : black_king_moved_;
                if (!king_moved && row == home && col == 4) {
                    bool h_moved = white ? white_rook_h_moved_ : black_rook_h_moved_;
                    if (!h_moved && at(home,7).type == PieceType::Rook && at(home,7).side == p.side && at(home,5).is_empty() && at(home,6).is_empty() && !attacked(home,5,opposite(p.side)) && !attacked(home,6,opposite(p.side))) add(home,4,home,6);
                    bool a_moved = white ? white_rook_a_moved_ : black_rook_a_moved_;
                    if (!a_moved && at(home,0).type == PieceType::Rook && at(home,0).side == p.side && at(home,1).is_empty() && at(home,2).is_empty() && at(home,3).is_empty() && !attacked(home,3,opposite(p.side)) && !attacked(home,2,opposite(p.side))) add(home,4,home,2);
                }
            }
        }
    }
    return moves;
}

bool Board::apply_unchecked(const Move& m) {
    if (!inside(m.from_row,m.from_col) || !inside(m.to_row,m.to_col)) return false;
    int from=m.from_row*8+m.from_col, to=m.to_row*8+m.to_col; Piece moving=squares_[from]; if (moving.is_empty()) return false;
    Piece captured=squares_[to];
    if (moving.type == PieceType::King && std::abs(m.to_col-m.from_col)==2) {
        int rook_from=m.to_col==6?m.from_row*8+7:m.from_row*8; int rook_to=m.to_col==6?m.from_row*8+5:m.from_row*8+3;
        squares_[rook_to]=squares_[rook_from]; squares_[rook_from]=Piece{};
    }
    if (moving.type == PieceType::Pawn && to == en_passant_square_ && captured.is_empty() && m.from_col != m.to_col) { int cap_row=m.to_row+(moving.side==ColorSide::White?1:-1); squares_[cap_row*8+m.to_col]=Piece{}; }
    if (moving.type == PieceType::King) (moving.side==ColorSide::White?white_king_moved_:black_king_moved_)=true;
    if (moving.type == PieceType::Rook) {
        if (from == 56) white_rook_a_moved_ = true;
        if (from == 63) white_rook_h_moved_ = true;
        if (from == 0) black_rook_a_moved_ = true;
        if (from == 7) black_rook_h_moved_ = true;
    }
    if (captured.type == PieceType::Rook) {
        if (to == 56) white_rook_a_moved_ = true;
        if (to == 63) white_rook_h_moved_ = true;
        if (to == 0) black_rook_a_moved_ = true;
        if (to == 7) black_rook_h_moved_ = true;
    }
    squares_[to]=moving; squares_[from]=Piece{};
    if (moving.type==PieceType::Pawn && (m.to_row==0 || m.to_row==7)) squares_[to].type=m.promotion;
    en_passant_square_=(moving.type==PieceType::Pawn && std::abs(m.to_row-m.from_row)==2)?((m.from_row+m.to_row)/2)*8+m.from_col:-1;
    side_to_move_=opposite(side_to_move_); return true;
}

std::vector<Move> Board::legal_moves() const {
    std::vector<Move> legal;
    for (const Move& m : pseudo_moves()) { Board copy=*this; if (copy.apply_unchecked(m) && !copy.in_check(side_to_move_)) legal.push_back(m); }
    return legal;
}
std::vector<Move> Board::legal_moves_from(int row,int col) const { auto all=legal_moves(); std::erase_if(all,[&](const Move& m){return m.from_row!=row||m.from_col!=col;}); return all; }
bool Board::is_checkmate() const { return in_check(side_to_move_) && legal_moves().empty(); }
bool Board::is_stalemate() const { return !in_check(side_to_move_) && legal_moves().empty(); }
bool Board::apply_move(const Move& move) { auto legal=legal_moves(); if(std::find(legal.begin(),legal.end(),move)==legal.end()) return false; return apply_unchecked(move); }
