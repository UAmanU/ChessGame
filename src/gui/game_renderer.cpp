#include "gui/game_renderer.h"
#include "config/colors_settings.h"
#include "config/window_settings.h"

namespace {
    constexpr int BOARD_SIZE=600, CELL=75, LEFT=60, TOP=56;
    const char* glyph(Piece p) {
        static const char* white[]={"","♙","♘","♗","♖","♕","♔"};
        static const char* black[]={"","♟","♞","♝","♜","♛","♚"};
        return p.side==ColorSide::White?white[static_cast<int>(p.type)]:black[static_cast<int>(p.type)];
    }
}
void GameRenderer::draw(const Board& board,int selected,const std::vector<Move>& moves,bool thinking) const {
    DrawRectangle(0,0,config::window::WIDTH,config::window::HEIGHT,config::colors::BACKGROUND);
    for(int row=0;row<8;++row) for(int col=0;col<8;++col) {
        int square=row*8+col; Rectangle rect{static_cast<float>(LEFT+col*CELL),static_cast<float>(TOP+row*CELL),static_cast<float>(CELL),static_cast<float>(CELL)};
        DrawRectangleRec(rect,((row+col)%2==0)?config::colors::BOARD_LIGHT:config::colors::BOARD_DARK);
        if(square==selected) DrawRectangleRec(rect,config::colors::HIGHLIGHT);
        for(const auto& move:moves) if(move.to_row==row&&move.to_col==col) {
            if(!board.at(row,col).is_empty()) DrawCircle(LEFT+col*CELL+CELL/2,TOP+row*CELL+CELL/2,32,Fade(RED,0.42f));
            else DrawCircle(LEFT+col*CELL+CELL/2,TOP+row*CELL+CELL/2,9,Fade(BLACK,0.35f));
        }
        Piece piece=board.at(row,col); if(!piece.is_empty()) {
            const char* text=glyph(piece); int size=54;
            Color color=piece.side==ColorSide::White?Color{250,250,245,255}:Color{35,38,43,255};
            DrawText(text,LEFT+col*CELL+CELL/2-MeasureText(text,size)/2,TOP+row*CELL+7,size,color);
        }
    }
    DrawText("Ход белых — игрок",690,120,22,config::colors::TEXT);
    DrawText("Ход чёрных — компьютер",690,160,20,config::colors::TEXT);
    DrawText("Esc — в меню",690,220,20,config::colors::TEXT);
    if(thinking) DrawText("Компьютер думает...",690,275,20,config::colors::ACCENT);
    DrawText("Вы играете белыми",690,340,22,config::colors::TEXT);
    (void)BOARD_SIZE;
}
