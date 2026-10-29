#include "chess/board.h"
#include "chess/engine.h"
#include <cassert>

int main() {
    Board board;
    assert(board.legal_moves().size() == 20);

    const Move e2_e4{6, 4, 4, 4};
    assert(board.apply_move(e2_e4));
    assert(board.at(4, 4).type == PieceType::Pawn);
    assert(board.side_to_move() == ColorSide::Black);
    assert(!board.apply_move(e2_e4));

    const Move e7_e5{1, 4, 3, 4};
    assert(board.apply_move(e7_e5));
    const Move g1_f3{7, 6, 5, 5};
    assert(board.apply_move(g1_f3));
    assert(board.legal_moves().size() > 0);

    ChessEngine engine(1);
    auto response = engine.find_best_move(board);
    assert(response.has_value());
    assert(board.apply_move(*response));
}
