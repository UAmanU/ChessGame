#include "config/assets_settings.h"
#include "raylib.h"
#include <algorithm>
#include <array>
#include <iostream>

int main() {
    constexpr std::array<int, 12> chess_pieces{
        0x2654, 0x2655, 0x2656, 0x2657, 0x2658, 0x2659,
        0x265A, 0x265B, 0x265C, 0x265D, 0x265E, 0x265F
    };

    int file_size = 0;
    unsigned char* font_data = LoadFileData(config::assets::CHESS_FONT_PATH, &file_size);
    if (font_data == nullptr) {
        std::cerr << "Could not load chess font asset.\n";
        return 1;
    }

    int glyph_count = 0;
    GlyphInfo* glyphs = LoadFontData(font_data, file_size, 96, chess_pieces.data(),
        static_cast<int>(chess_pieces.size()), FONT_DEFAULT, &glyph_count);
    const bool all_glyphs_found = glyphs != nullptr && glyph_count == static_cast<int>(chess_pieces.size()) &&
        std::all_of(chess_pieces.begin(), chess_pieces.end(), [&](int codepoint) {
            return std::any_of(glyphs, glyphs + glyph_count,
                [codepoint](const GlyphInfo& glyph) { return glyph.value == codepoint && glyph.image.data != nullptr; });
        });

    if (glyphs != nullptr) UnloadFontData(glyphs, glyph_count);
    UnloadFileData(font_data);
    if (!all_glyphs_found) {
        std::cerr << "Chess font does not contain all 12 chess symbols.\n";
        return 2;
    }
    return 0;
}
