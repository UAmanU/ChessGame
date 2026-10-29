#include "launcher/programm_launcher.h"
#include "config/menu_settings.h"
#include "config/colors_settings.h"
#include "config/window_settings.h"

ProgrammLauncher::ProgrammLauncher() : game_(std::make_unique<Game>()),menu_(std::make_unique<Menu>()) {}
ProgrammLauncher::~ProgrammLauncher() { if(IsWindowReady()) CloseWindow(); }
void ProgrammLauncher::start() {
    InitWindow(config::window::WIDTH,config::window::HEIGHT,config::window::TITLE);
    SetTargetFPS(config::window::TARGET_FPS);
    while(!WindowShouldClose()&&game_state_!=GameState::Exit) {
        if(game_state_==GameState::Menu) menu_->update(game_state_);
        else if(game_state_==GameState::Playing) game_->update(game_state_,player_won_,draw_);
        else if(game_state_==GameState::Result) menu_->update(game_state_,true,nullptr);
        BeginDrawing();
        ClearBackground(config::colors::BACKGROUND);
        if(game_state_==GameState::Menu) menu_->draw();
        else if(game_state_==GameState::Playing) game_->draw();
        else if(game_state_==GameState::Result) menu_->draw(true,draw_?config::menu::DRAW_MESSAGE:(player_won_?config::menu::WIN_MESSAGE:config::menu::LOSE_MESSAGE));
        EndDrawing();
    }
}
