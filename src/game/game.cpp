#include "game/game.h"
#include "config/window_settings.h"

namespace
{
    constexpr int LEFT = 60, TOP = 56, CELL = 75;
}
Game::Game() : engine_(3), renderer_(std::make_unique<GameRenderer>()) {}
void Game::reset()
{
    board_.reset();
    selected_square_ = -1;
    selected_moves_.clear();
    thinking_ = false;
}
void Game::draw() const { renderer_->draw(board_, selected_square_, selected_moves_, thinking_); }
void Game::update(GameState &state, bool &player_won, bool &draw)
{
    if (IsKeyPressed(KEY_ESCAPE))
    {
        state = GameState::Menu;
        reset();
        return;
    }
    if (board_.is_checkmate() || board_.is_stalemate())
    {
        draw = board_.is_stalemate();
        player_won = board_.side_to_move() == ColorSide::Black;
        state = GameState::Result;
        return;
    }
    if (board_.side_to_move() == ColorSide::Black)
    {
        // Keep the interface responsive by showing a brief thinking state before searching.
        if (!thinking_)
        {
            thinking_ = true;
            ai_started_at_ = GetTime();
            return;
        }
        if (GetTime() - ai_started_at_ < 0.12)
            return;
        auto move = engine_.find_best_move(board_);
        if (move)
            (void)board_.apply_move(*move);
        thinking_ = false;
        return;
    }
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        return;
    Vector2 mouse = GetMousePosition();
    int col = static_cast<int>((mouse.x - LEFT) / CELL), row = static_cast<int>((mouse.y - TOP) / CELL);
    if (mouse.x < LEFT || mouse.y < TOP || row >= 8 || col >= 8)
    {
        selected_square_ = -1;
        selected_moves_.clear();
        return;
    }
    if (selected_square_ >= 0)
        for (const auto &move : selected_moves_)
            if (move.to_row == row && move.to_col == col)
            {
                (void)board_.apply_move(move);
                selected_square_ = -1;
                selected_moves_.clear();
                return;
            }
    Piece piece = board_.at(row, col);
    if (!piece.is_empty() && piece.side == ColorSide::White)
    {
        selected_square_ = row * 8 + col;
        selected_moves_ = board_.legal_moves_from(row, col);
    }
    else
    {
        selected_square_ = -1;
        selected_moves_.clear();
    }
}
