#pragma once
#include "chess/board.h"
#include "raylib.h"
class GameRenderer
{
public:
    GameRenderer() = default;
    ~GameRenderer();
    GameRenderer(const GameRenderer &) = delete;
    GameRenderer &operator=(const GameRenderer &) = delete;

    // Call after InitWindow so Raylib can upload the generated font atlas to the GPU.
    void load_assets();
    void draw(const Board &board, int selected_square, const std::vector<Move> &moves, bool thinking) const;

private:
    Font chess_font_{};
};
