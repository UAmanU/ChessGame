#pragma once

enum class PieceType { None, Pawn, Knight, Bishop, Rook, Queen, King };
enum class ColorSide { White, Black };

struct Piece {
    PieceType type{PieceType::None};
    ColorSide side{ColorSide::White};
    [[nodiscard]] bool is_empty() const noexcept { return type == PieceType::None; }
};
