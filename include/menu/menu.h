#pragma once
#include "game/game_state.h"
#include "gui/menu_renderer.h"
#include <memory>
class Menu {
public:
    Menu();
    void update(GameState& state, bool result_screen = false, const char* result = nullptr);
    void draw(bool result_screen = false, const char* result = nullptr) const;
private:
    std::unique_ptr<MenuRenderer> renderer_;
};
