#pragma once
#include "chess/piece.h"

struct Move {
    int from_row{};
    int from_col{};
    int to_row{};
    int to_col{};
    PieceType promotion{PieceType::Queen};
    bool operator==(const Move&) const = default;
};
