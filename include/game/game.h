#pragma once
#include "chess/board.h"
#include "chess/engine.h"
#include "game/game_state.h"
#include "gui/game_renderer.h"
#include <memory>
#include <optional>
#include <future>
#include <cstdint>
class Game
{
public:
    Game();
    void load_assets();
    void update(GameState &state, bool &player_won, bool &draw);
    void draw() const;
    void reset();

private:
    Board board_;
    ChessEngine engine_;
    std::unique_ptr<GameRenderer> renderer_;
    int selected_square_{-1};
    std::vector<Move> selected_moves_;
    std::vector<Move> move_history_;
    bool thinking_{false};
    std::future<std::optional<Move>> ai_future_;
    std::uint64_t ai_position_key_{0};
};
