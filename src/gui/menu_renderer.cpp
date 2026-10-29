#include "gui/menu_renderer.h"
#include "config/colors_settings.h"
#include "config/menu_settings.h"
#include "config/window_settings.h"

void MenuRenderer::draw_menu() const {
    DrawText("CHESS", config::window::WIDTH / 2 - 112, 170, 72, config::colors::TEXT);
    DrawText("Шахматы против компьютера", config::window::WIDTH / 2 - 175, 255, 24, config::colors::TEXT);
    DrawRectangleRounded({config::window::WIDTH/2.0f-130,350,260,70},0.18f,8,config::colors::ACCENT);
    DrawText(config::menu::PLAY_LABEL,config::window::WIDTH/2-48,369,30,config::colors::TEXT);
    DrawRectangleRounded({config::window::WIDTH/2.0f-130,445,260,70},0.18f,8,Color{94,102,115,255});
    DrawText(config::menu::EXIT_LABEL,config::window::WIDTH/2-48,464,30,config::colors::TEXT);
}
void MenuRenderer::draw_result(const char* message) const {
    DrawRectangle(0,0,config::window::WIDTH,config::window::HEIGHT,config::colors::BACKGROUND);
    DrawText(message,config::window::WIDTH/2-MeasureText(message,44)/2,250,44,config::colors::TEXT);
    DrawText("Нажмите, чтобы вернуться в меню",config::window::WIDTH/2-215,340,24,config::colors::TEXT);
}
