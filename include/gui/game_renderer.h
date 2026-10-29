#pragma once
#include "chess/board.h"
#include "raylib.h"
class GameRenderer {
public:
    void draw(const Board& board, int selected_square, const std::vector<Move>& moves, bool thinking) const;
};
