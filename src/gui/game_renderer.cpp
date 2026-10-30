#include "gui/game_renderer.h"
#include "config/assets_settings.h"
#include "config/colors_settings.h"
#include "config/window_settings.h"
#include <algorithm>
#include <array>
#include <filesystem>

namespace {
    constexpr int CELL = 75, LEFT = 60, TOP = 56;
    constexpr std::array<int, 12> CHESS_CODEPOINTS{
        0x2654, 0x2655, 0x2656, 0x2657, 0x2658, 0x2659,
        0x265A, 0x265B, 0x265C, 0x265D, 0x265E, 0x265F
    };

    int glyph_codepoint(Piece piece) {
        constexpr int white[] = {0, 0x2659, 0x2658, 0x2657, 0x2656, 0x2655, 0x2654};
        constexpr int black[] = {0, 0x265F, 0x265E, 0x265D, 0x265C, 0x265B, 0x265A};
        return piece.side == ColorSide::White ? white[static_cast<int>(piece.type)] : black[static_cast<int>(piece.type)];
    }

    const char* fallback_label(Piece piece) {
        constexpr const char* white[] = {"", "P", "N", "B", "R", "Q", "K"};
        constexpr const char* black[] = {"", "p", "n", "b", "r", "q", "k"};
        return piece.side == ColorSide::White ? white[static_cast<int>(piece.type)] : black[static_cast<int>(piece.type)];
    }
}

GameRenderer::~GameRenderer() {
    if (chess_font_.texture.id != 0 && IsWindowReady()) UnloadFont(chess_font_);
}

void GameRenderer::load_assets() {
    const std::filesystem::path font_path = std::filesystem::path(GetApplicationDirectory()) / config::assets::CHESS_FONT_PATH;
    if (!FileExists(font_path.string().c_str())) {
        TraceLog(LOG_WARNING, "Chess font file was not found: %s", font_path.string().c_str());
        return;
    }

    Font loaded_font = LoadFontEx(font_path.string().c_str(), 96, CHESS_CODEPOINTS.data(), static_cast<int>(CHESS_CODEPOINTS.size()));
    const bool contains_all_pieces = loaded_font.glyphs != nullptr && std::all_of(
        CHESS_CODEPOINTS.begin(), CHESS_CODEPOINTS.end(), [&](int codepoint) {
            return std::any_of(loaded_font.glyphs, loaded_font.glyphs + loaded_font.glyphCount,
                [codepoint](const GlyphInfo& glyph) { return glyph.value == codepoint; });
        });
    if (!contains_all_pieces || loaded_font.texture.id == 0) {
        // LoadFontEx may return Raylib's built-in font after failure; never unload that shared font.
        if (loaded_font.texture.id != 0 && loaded_font.texture.id != GetFontDefault().texture.id) UnloadFont(loaded_font);
        TraceLog(LOG_WARNING, "Chess font did not provide all required chess symbols: %s", font_path.string().c_str());
        return;
    }
    chess_font_ = loaded_font;
}

void GameRenderer::draw(const Board& board, int selected, const std::vector<Move>& moves, bool thinking) const {
    DrawRectangle(0, 0, config::window::WIDTH, config::window::HEIGHT, config::colors::BACKGROUND);
    for (int row = 0; row < 8; ++row) for (int col = 0; col < 8; ++col) {
        int square = row * 8 + col;
        Rectangle rect{static_cast<float>(LEFT + col * CELL), static_cast<float>(TOP + row * CELL), static_cast<float>(CELL), static_cast<float>(CELL)};
        DrawRectangleRec(rect, ((row + col) % 2 == 0) ? config::colors::BOARD_LIGHT : config::colors::BOARD_DARK);
        if (square == selected) DrawRectangleRec(rect, config::colors::HIGHLIGHT);
        for (const auto& move : moves) if (move.to_row == row && move.to_col == col) {
            if (!board.at(row, col).is_empty()) DrawCircle(LEFT + col * CELL + CELL / 2, TOP + row * CELL + CELL / 2, 32, Fade(RED, 0.42f));
            else DrawCircle(LEFT + col * CELL + CELL / 2, TOP + row * CELL + CELL / 2, 9, Fade(BLACK, 0.35f));
        }
        Piece piece = board.at(row, col);
        if (!piece.is_empty()) {
            Color color = piece.side == ColorSide::White ? Color{250, 250, 245, 255} : Color{35, 38, 43, 255};
            if (chess_font_.texture.id != 0) {
                constexpr float font_size = 62.0f;
                const int codepoint = glyph_codepoint(piece);
                const GlyphInfo glyph = GetGlyphInfo(chess_font_, codepoint);
                const float width = glyph.advanceX * font_size / chess_font_.baseSize;
                DrawTextCodepoint(chess_font_, codepoint,
                    {LEFT + col * CELL + (CELL - width) / 2.0f, TOP + row * CELL + 5.0f}, font_size, color);
            } else {
                const char* label = fallback_label(piece);
                constexpr int fallback_size = 42;
                DrawText(label, LEFT + col * CELL + CELL / 2 - MeasureText(label, fallback_size) / 2,
                    TOP + row * CELL + 15, fallback_size, color);
            }
        }
    }
    DrawText("Ход белых — игрок", 690, 120, 22, config::colors::TEXT);
    DrawText("Ход чёрных — компьютер", 690, 160, 20, config::colors::TEXT);
    DrawText("Esc — в меню", 690, 220, 20, config::colors::TEXT);
    if (thinking) DrawText("Компьютер думает...", 690, 275, 20, config::colors::ACCENT);
    DrawText("Вы играете белыми", 690, 340, 22, config::colors::TEXT);
}
