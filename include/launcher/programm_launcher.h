#pragma once
#include "game/game.h"
#include "menu/menu.h"
#include <memory>
class ProgrammLauncher {
public:
    ProgrammLauncher();
    ~ProgrammLauncher();
    void start();
private:
    std::unique_ptr<Game> game_;
    std::unique_ptr<Menu> menu_;
    GameState game_state_{GameState::Menu};
    bool player_won_{false};
    bool draw_{false};
};
