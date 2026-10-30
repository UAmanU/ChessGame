#pragma once
#include "chess/move.h"
#include <optional>
#include <vector>

class OpeningBook {
public:
    // Return a familiar book reply only when the recorded game prefix matches exactly.
    [[nodiscard]] static std::optional<Move> find_move(
        const std::vector<Move>& history,
        const std::vector<Move>& legal_moves);
};
