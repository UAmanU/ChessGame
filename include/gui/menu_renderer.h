#pragma once
#include "raylib.h"
class MenuRenderer {
public:
    void draw_menu() const;
    void draw_result(const char* message) const;
};
