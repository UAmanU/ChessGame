#include "menu/menu.h"
#include "config/window_settings.h"

Menu::Menu() : renderer_(std::make_unique<MenuRenderer>()) {}
void Menu::update(GameState& state,bool result_screen,const char*) {
    if(!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return;
    Vector2 mouse=GetMousePosition();
    if(result_screen) { state=GameState::Menu; return; }
    if(mouse.x>=config::window::WIDTH/2-130&&mouse.x<=config::window::WIDTH/2+130) {
        if(mouse.y>=350&&mouse.y<=420) state=GameState::Playing;
        else if(mouse.y>=445&&mouse.y<=515) state=GameState::Exit;
    }
}
void Menu::draw(bool result_screen,const char* result) const { if(result_screen) renderer_->draw_result(result?result:""); else renderer_->draw_menu(); }
