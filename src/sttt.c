#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "ttt.h"

#include <rlgl.h>

#define GLSL_VERSION 330

typedef struct {
    int m_size;
    int width;
    int active_cell;
    float runTime;
    bool m_gameOver;
    Turn m_turn;
    ttt_state** m_state;
    Winner m_winner;

    int win_start, win_end;

    Shader confetti;
    int confetti_res_loc;
    int confetti_time_loc;
} s_ttt_state;

s_ttt_state* state = NULL;

#define ind_to_coord(ind, x, y)        \
    int x = (ind) / (state->m_size);   \
    int y = (ind) % (state->m_size);

#define coord_to_ind(x, y) ((x) * state->m_size + (y))

void s_ttt_init(int size, int width, int height) {
    state = (s_ttt_state*)malloc(sizeof(s_ttt_state));
    memset(state, 0, sizeof(*state));
    state->m_state = malloc(size * size * sizeof(ttt_state*));
    state->m_size = size;
    state->m_gameOver = false;
    state->active_cell = -1;
    state->width = width;

    float cellSize = (float)width / size;
    ttt_state** states = state->m_state;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            ttt_state* new_ttt_state = (ttt_state*)malloc(sizeof(ttt_state));
            assert(new_ttt_state != NULL && "Unable to allocate memory");
            memset(new_ttt_state, 0, sizeof(*new_ttt_state));

            new_ttt_state->m_size = size;
            new_ttt_state->width = cellSize - 4;

            float x_off = i * cellSize, y_off = j * cellSize;
            new_ttt_state->x_offset = x_off + 2;
            new_ttt_state->y_offset = y_off + 2;

            new_ttt_state->m_state = (Turn*)malloc(size * size * sizeof(Turn));
            assert(new_ttt_state->m_state != NULL && "Unable to allocate memory");
            for (int i = 0; i < size * size; i++) {
                new_ttt_state->m_state[i] = Turn_None;
            }
            states[coord_to_ind(i, j)] = new_ttt_state;
        }
    }

    state->confetti = LoadShader(0, TextFormat("./resources/shaders/glsl%d/confetti.fs", GLSL_VERSION));
    state->confetti_time_loc = GetShaderLocation(state->confetti, "time");
    state->confetti_res_loc = GetShaderLocation(state->confetti, "resolution");
}

void s_ttt_reset() {
    state->runTime = 0;
    for (int i = 0; i < state->m_size * state->m_size; i++) {
        ttt_reset(state->m_state[i]);
    }
    state->m_turn = Turn_Cross;
    state->m_gameOver = false;
}

bool s_walk(int x, int y, int dx, int dy) {
    Turn turn = state->m_turn;
    int size = state->m_size;
    int cnt = 1;
    while (true) {
        int nx = x + dx, ny = y + dy;
        if (nx >= 0 && nx < size && ny >= 0 && ny < size) {
            int currIndex = x * size + y;
            int nextIndex = nx * size + ny;
            ttt_state* curr = state->m_state[currIndex];
            ttt_state* next = state->m_state[nextIndex];
            if(!(curr->m_gameOver && next->m_gameOver)) {
                break;
            }
            if (curr->m_winner != Turn_None && curr->m_winner == next->m_winner) {
                x = nx;
                y = ny;
                cnt++;
            }
            else {
                break;
            }
        }
        else {
            break;
        }
    }
    return cnt == size;
};

void s_ttt_end_game(Winner winner) {
    if (state->m_gameOver) {
        return;
    }
    printf("GAME ENDED: %d ", winner);
    switch (winner) {
    case Winner_Circle: {
        printf("Circle \n");
        break;
    };
    case Winner_Cross: {
        printf("Cross \n");
        break;
    };
    default:
        printf("Draw \n");
    }
    state->m_winner = winner;
    state->m_gameOver = true;
    state->runTime = 0;
}

void s_ttt_check_winner() {
    int dx[ ] = { 1, 1, 0, -1 }, dy[ ] = { 0, 1, 1, 1 };
    for (int k = 0; k < 4; k++) {
        for (int i = 0; i < state->m_size; i++) {
            bool gameWon = s_walk(i, 0, dx[k], dy[k]);
            if (gameWon == true) {
                int ind = coord_to_ind(i, 0);
                state->win_start = ind;
                state->win_end = coord_to_ind(i + dx[k] * 2, dy[k] * 2);
                if (state->m_winner == Turn_Circle) {
                    s_ttt_end_game(Winner_Circle);
                }
                else {
                    s_ttt_end_game(Winner_Cross);
                }
            }
            gameWon = s_walk(0, i, dx[k], dy[k]);
            if (gameWon == true) {
                int ind = coord_to_ind(0, i);
                state->win_start = ind;
                state->win_end = coord_to_ind(dx[k] * 2, i + dy[k] * 2);
                if (state->m_winner == Turn_Circle) {
                    s_ttt_end_game(Winner_Circle);
                }
                else {
                    s_ttt_end_game(Winner_Cross);
                }
            }
        }
    }

    // if (!state->m_gameOver) {
    //     bool draw = true;
    //     for (int i = 0; i < state->m_size * state->m_size; i++) {
    //         draw &= state->m_state[i] != Turn_None;
    //     }
    //     if (draw) {
    //         s_ttt_end_game(Winner_Drawn);
    //     }
    // }
}

void s_ttt_set_active(int ind) {
    if(state->m_state[ind]->m_gameOver) {
        state->active_cell = -1;
    }
    else {
        state->active_cell = ind;
    }
}

void s_ttt_clicked(Vector2 pos) {
    if (state->m_gameOver) {
        return;
    }
    float cellSize = (float)state->width / state->m_size;
    int clicked_ind = (state->m_size * (int)(pos.x / cellSize)) + pos.y / cellSize;
    if ((state->active_cell != -1 && clicked_ind == state->active_cell) ||
        (state->active_cell == -1)) {
        int clicked = ttt_clicked(state->m_state[clicked_ind], pos, state->m_turn);
        if (clicked != -1) {
            s_ttt_set_active(clicked);
            state->m_turn = state->m_turn == Turn_Circle ? Turn_Cross : Turn_Circle;
        }
    }
    s_ttt_check_winner();
}

void s_ttt_draw(float dt) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        s_ttt_clicked(GetMousePosition());
    }

    float cellSize = (float)GetScreenWidth() / state->m_size;
    Color lineCol = PINK;

    Color clear_color = { 120, 130, 130, 255 };

    // if (state->m_gameOver && state->m_winner != Winner_Drawn) {
    //     state->runTime += dt * 0.5;
    //     SetShaderValue(state->confetti, state->confetti_time_loc, &state->runTime, SHADER_UNIFORM_FLOAT);

    //     float res[2] = { (float)GetScreenWidth(), (float)GetScreenHeight() };
    //     SetShaderValue(state->confetti, state->confetti_res_loc, res, SHADER_UNIFORM_VEC2);

    //     // Texture2D texture = { rlGetTextureIdDefault(), 1, 1, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 };
    //     // BeginShaderMode(state->confetti);
    //     // // DrawTexture(texture, 5, 5, RED);
    //     // DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), CLITERAL(Color){255, 0, 0, 0});
    //     // EndShaderMode();
    // }
    // else {
    ClearBackground(clear_color);
    // }
    for (int i = 0; i < state->m_size; i++) {
        for (int j = 0; j < state->m_size; j++) {
            float cell_x = i * cellSize, cell_y = j * cellSize;
            Rectangle rec = { cell_x, cell_y, cellSize, cellSize };

            // DrawRectangleLinesEx(rec, 5, lineCol);

            int ind = coord_to_ind(i, j);
            // Draw ttt
            ttt_draw(state->m_state[ind], dt);

            // if (state->m_state[ind]->m_winner == Turn_Circle) {
            //     Vector2 center = cell_center(cell_x, cell_y, cellSize);
            //     float outerRadius = cellSize * 0.8 * 0.5;
            //     float innerRadius = cellSize * 0.7 * 0.5;
            //     DrawRing(center, innerRadius, outerRadius, 0, 360, 1, PURPLE);
            // }
            // else if (state->m_state[ind]->m_winner == Turn_Cross) {
            //     Vector2 pos1 = { (float)(cell_x + 0.2 * cellSize),
            //                     (float)(cell_y + 0.2 * cellSize) };
            //     Vector2 pos2 = { (float)(pos1.x + 0.6 * cellSize),
            //                     (float)(pos1.y + 0.6 * cellSize) };
            //     Vector2 pos3 = { pos2.x, pos1.y };
            //     Vector2 pos4 = { pos1.x, pos2.y };
            //     Color lineCol = { 0xE1, 0xFF, 0x2F, 0xFF };
            //     DrawLineEx(pos1, pos2, 5, lineCol);
            //     DrawLineEx(pos3, pos4, 5, lineCol);
            // }
        }
    }

    if (state->m_gameOver && state->m_winner != Winner_Drawn) {
        int start_i = state->win_start, end_i = state->win_end;
        ind_to_coord(start_i, start_x, start_y);
        ind_to_coord(end_i, end_x, end_y);
        Vector2 start_pos = cell_center(start_x * cellSize, start_y * cellSize, cellSize);
        Vector2 end_pos = cell_center(end_x * cellSize, end_y * cellSize, cellSize);
        DrawLineEx(start_pos, end_pos, 10, (Color) { 255, 255, 255, 190 });
    }
    // BeginShaderMode(state->confetti);
    // // DrawTexture(texture, 5, 5, RED);
    // DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), CLITERAL(Color){255, 0, 0, 0});
    // EndShaderMode();
}

// void* s_ttt_pre_reload() {
//     UnloadShader(state->confetti);
//     s_ttt_state* ptr = (s_ttt_state*)malloc(sizeof(*state));
//     memcpy(ptr, state, sizeof(*state));



//     return ptr;
// }

// void s_ttt_post_reload(void* ptr) {
//     state = (ttt_state*)malloc(sizeof(*state));
//     memcpy(state, ptr, sizeof(*state));
//     state->confetti = LoadShader(0, TextFormat("./resources/shaders/glsl%d/confetti.fs", GLSL_VERSION));
//     state->confetti_time_loc = GetShaderLocation(state->confetti, "time");
//     state->confetti_res_loc = GetShaderLocation(state->confetti, "resolution");
//     free(ptr);
// }
