#ifndef TTT_H_
#define TTT_H_

#include <raylib.h>
#include <stdlib.h>
#include <stdio.h>

typedef enum Turn {
    Turn_Cross,
    Turn_Circle,
    Turn_None = -1
} Turn;

typedef enum Winner {
    Winner_Cross,
    Winner_Circle,
    Winner_Drawn
} Winner;

typedef struct {
    int m_size;
    float width;
    float runTime;
    bool m_gameOver;
    Turn* m_state;

    Winner m_winner;
    int win_start, win_end;

    int x_offset, y_offset;

    Shader confetti;
    int confetti_res_loc;
    int confetti_time_loc;
} ttt_state;

#define LIST_OF_FUNCS                          \
    FUNC(ttt_draw, void, ttt_state*, float)    \
    FUNC(s_ttt_reset, void, void)              \
    FUNC(s_ttt_draw, void, float)              \
    FUNC(s_ttt_init, void, int, int, int)      \
    FUNC(s_ttt_pre_reload, void*, void)        \
    FUNC(s_ttt_post_reload, void, void*)       \

#define FUNC(func, ret, ...) typedef ret func##_t(__VA_ARGS__);
LIST_OF_FUNCS
#undef FUNC


int ttt_clicked(ttt_state*, Vector2, Turn);
void ttt_check_winner(ttt_state*);
void ttt_reset(ttt_state*);

// Explicitly declaring, emcc errors on implicit declaration
#ifdef __EMSCRIPTEN__
void ttt_draw(ttt_state*, float);
#endif 

// Returns the center of cell with top left coords (x, y) and cell size cellSize
static Vector2 cell_center(float x, float y, int cellSize) {
    return CLITERAL(Vector2) { x + cellSize / 2, y + cellSize / 2 };
}

#endif // TTT_H_
