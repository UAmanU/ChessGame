#include "game/game.h"
#include "config/engine_settings.h"
#include "config/window_settings.h"
#include <chrono>
#include <utility>

namespace
{
    constexpr int LEFT = 60, TOP = 56, CELL = 75;
}
Game::Game() : engine_(config::engine::SEARCH_DEPTH), renderer_(std::make_unique<GameRenderer>()) {}
void Game::load_assets() { renderer_->load_assets(); }
void Game::reset()
{
    board_.reset();
    selected_square_ = -1;
    selected_moves_.clear();
    move_history_.clear();
    thinking_ = false;
    ai_position_key_ = 0;
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
        if (ai_future_.valid())
        {
            if (ai_future_.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready)
            {
                thinking_ = true;
                return;
            }

            const auto move = ai_future_.get();
            thinking_ = false;
            if (ai_position_key_ == board_.position_key())
            {
                if (move && board_.apply_move(*move)) move_history_.push_back(*move);
                return;
            }
        }

        thinking_ = true;
        const Board position = board_;
        const std::vector<Move> history = move_history_;
        const ChessEngine engine = engine_;
        ai_position_key_ = position.position_key();
        ai_future_ = std::async(std::launch::async, [engine, position, history]() {
            return engine.find_best_move(position, history);
        });
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
                if (board_.apply_move(move)) move_history_.push_back(move);
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
