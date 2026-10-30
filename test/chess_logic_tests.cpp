#include "chess/board.h"
#include "chess/engine.h"
#include <cassert>
#include <vector>

int main() {
    Board board;
    assert(board.legal_moves().size() == 20);

    const Move e2_e4{6, 4, 4, 4};
    assert(board.apply_move(e2_e4));
    assert(board.at(4, 4).type == PieceType::Pawn);
    assert(board.side_to_move() == ColorSide::Black);
    assert(!board.apply_move(e2_e4));

    ChessEngine opening_engine(3);
    std::vector<Move> move_history{e2_e4};
    const auto search_response_to_e4 = opening_engine.find_best_move(board);
    assert(search_response_to_e4.has_value());
    assert(*search_response_to_e4 != (Move{1, 0, 3, 0})); // The evaluation must also reject ...a5 without a book hit.

    const auto response_to_e4 = opening_engine.find_best_move(board, move_history);
    assert(response_to_e4.has_value());
    assert(*response_to_e4 != (Move{1, 0, 3, 0})); // Do not waste Black's first move on ...a5.
    assert(*response_to_e4 == (Move{1, 4, 3, 4})); // Meet the central pawn with ...e5.
    assert(board.apply_move(*response_to_e4));
    move_history.push_back(*response_to_e4);

    const Move g1_f3{7, 6, 5, 5};
    assert(board.apply_move(g1_f3));
    move_history.push_back(g1_f3);
    assert(board.legal_moves().size() > 0);

    const auto response_to_nf3 = opening_engine.find_best_move(board, move_history);
    assert(response_to_nf3.has_value());
    assert(*response_to_nf3 == (Move{0, 1, 2, 2})); // Develop the queenside knight with ...Nc6.

    ChessEngine engine(1);
    auto response = engine.find_best_move(board); // Exercise Minimax after the opening-book line.
    assert(response.has_value());
    assert(board.apply_move(*response));

    // After 1. e4 d5 2. exd5 Qxd5, Nc3 attacks the exposed queen with tempo.
    Board queen_tactic;
    for (const Move move : std::vector<Move>{{6, 4, 4, 4}, {1, 3, 3, 3}, {4, 4, 3, 3}, {0, 3, 3, 3}}) {
        assert(queen_tactic.apply_move(move));
    }
    ChessEngine tactical_engine(3);
    const auto queen_attack = tactical_engine.find_best_move(queen_tactic);
    assert(queen_attack.has_value());
    assert(*queen_attack == (Move{7, 1, 5, 2}));

    // The engine should recognize the immediate Fool's Mate rather than settle for material.
    Board mate_in_one;
    for (const Move move : std::vector<Move>{{6, 5, 5, 5}, {1, 4, 3, 4}, {6, 6, 4, 6}}) {
        assert(mate_in_one.apply_move(move));
    }
    const auto mating_move = ChessEngine(2).find_best_move(mate_in_one);
    assert(mating_move.has_value());
    assert(*mating_move == (Move{0, 3, 4, 7}));
    assert(mate_in_one.apply_move(*mating_move));
    assert(mate_in_one.in_check(ColorSide::White));
    assert(mate_in_one.legal_moves().empty());
}
