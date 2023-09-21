#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "ttt.h"

#include <rlgl.h>

#define GLSL_VERSION 330

#define ind_to_coord(ind, x, y, _state) \
    int x = (ind) / (_state->m_size);   \
    int y = (ind) % (_state->m_size);

#define coord_to_ind(x, y, _state) ((int)(x) * m_state->m_size + (int)(y))


void ttt_reset(ttt_state* m_state) {
    m_state->runTime = 0;
    for (int i = 0; i < m_state->m_size * m_state->m_size; i++) {
        m_state->m_state[i] = Turn_None;
    }
    m_state->m_gameOver = false;
}

bool walk(ttt_state* m_state, int x, int y, int dx, int dy) {
    Turn* state = m_state->m_state;
    int size = m_state->m_size;
    int cnt = 1;
    while (true) {
        int nx = x + dx, ny = y + dy;
        if (nx >= 0 && nx < size && ny >= 0 && ny < size) {
            int currIndex = x * size + y;
            int nextIndex = nx * size + ny;
            if (state[currIndex] != Turn_None && state[currIndex] == state[nextIndex]) {
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

void ttt_end_game(ttt_state* m_state, Winner winner) {
    if (m_state->m_gameOver) {
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
    m_state->m_winner = winner;
    m_state->m_gameOver = true;
}

void ttt_check_winner(ttt_state* m_state) {
    int dx[] = { 1, 1, 0, -1 }, dy[] = { 0, 1, 1, 1 };
    for (int k = 0; k < 4; k++) {
        for (int i = 0; i < m_state->m_size; i++) {
            bool gameWon = walk(m_state, i, 0, dx[k], dy[k]);
            if (gameWon == true) {
                int ind = coord_to_ind(i, 0, m_state);
                m_state->win_start = ind;
                m_state->win_end = coord_to_ind(i + dx[k] * 2, dy[k] * 2, m_state);
                if (m_state->m_state[ind] == Turn_Circle) {
                    ttt_end_game(m_state, Winner_Circle);
                }
                else {
                    ttt_end_game(m_state, Winner_Cross);
                }
            }
            gameWon = walk(m_state, 0, i, dx[k], dy[k]);
            if (gameWon == true) {
                int ind = coord_to_ind(0, i, m_state);
                m_state->win_start = ind;
                m_state->win_end = coord_to_ind(dx[k] * 2, i + dy[k] * 2, m_state);
                if (m_state->m_state[ind] == Turn_Circle) {
                    ttt_end_game(m_state, Winner_Circle);
                }
                else {
                    ttt_end_game(m_state, Winner_Cross);
                }
            }
        }
    }

    if (!m_state->m_gameOver) {
        bool draw = true;
        for (int i = 0; i < m_state->m_size * m_state->m_size; i++) {
            draw &= m_state->m_state[i] != Turn_None;
        }
        if (draw) {
            ttt_end_game(m_state, Winner_Drawn);
        }
    }
}

int ttt_clicked(ttt_state* m_state, Vector2 pos, Turn turn) {
    if (m_state->m_gameOver) {
        return false;
    }
    int ret = -1;
    float cellSize = (float)m_state->width / m_state->m_size;

    float x_d = pos.x - m_state->x_offset;
    float y_d = pos.y - m_state->y_offset;
    int clicked_ind = coord_to_ind(x_d / cellSize, y_d / cellSize, m_state);

    if (m_state->m_state[clicked_ind] == Turn_None) {
        m_state->m_state[clicked_ind] = turn;
        ttt_check_winner(m_state);
        return clicked_ind;
    }

    return -1;
}

void ttt_draw(ttt_state* m_state, float dt) {
    float cellSize = (float)m_state->width / m_state->m_size;
    Color lineCol = PINK;

    Color clear_color = { 120, 130, 130, 255 };

    // if (m_state->m_gameOver && m_state->m_winner != Winner_Drawn) {
    //     m_state->runTime += dt * 0.5;
    //     SetShaderValue(m_state->confetti, m_state->confetti_time_loc, &m_state->runTime, SHADER_UNIFORM_FLOAT);

    //     float res[2] = { (float)GetScreenWidth(), (float)GetScreenHeight() };
    //     SetShaderValue(m_state->confetti, m_state->confetti_res_loc, res, SHADER_UNIFORM_VEC2);

    //     // Texture2D texture = { rlGetTextureIdDefault(), 1, 1, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 };
    //     // BeginShaderMode(m_state->confetti);
    //     // // DrawTexture(texture, 5, 5, RED);
    //     // DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), CLITERAL(Color){255, 0, 0, 0});
    //     // EndShaderMode();
    // }
    // else {
    // ClearBackground(clear_color);
    // }
    for (int i = 0; i < m_state->m_size; i++) {
        for (int j = 0; j < m_state->m_size; j++) {
            float cell_x = m_state->x_offset + i * cellSize;
            int cell_y = m_state->y_offset + j * cellSize;
            Rectangle rec = { cell_x, cell_y, cellSize, cellSize };

            DrawRectangleLinesEx(rec, 3, lineCol);

            int ind = coord_to_ind(i, j, m_state);
            if (m_state->m_state[ind] == Turn_Circle) {
                Vector2 center = cell_center(cell_x, cell_y, cellSize);
                float outerRadius = cellSize * 0.8 * 0.5;
                float innerRadius = cellSize * 0.7 * 0.5;
                DrawRing(center, innerRadius, outerRadius, 0, 360, 1, PURPLE);
            }
            else if (m_state->m_state[ind] == Turn_Cross) {
                Vector2 pos1 = { (float)(cell_x + 0.2 * cellSize),
                                (float)(cell_y + 0.2 * cellSize) };
                Vector2 pos2 = { (float)(pos1.x + 0.6 * cellSize),
                                (float)(pos1.y + 0.6 * cellSize) };
                Vector2 pos3 = { pos2.x, pos1.y };
                Vector2 pos4 = { pos1.x, pos2.y };
                Color lineCol = { 0xE1, 0xFF, 0x2F, 0xFF };
                DrawLineEx(pos1, pos2, 5, lineCol);
                DrawLineEx(pos3, pos4, 5, lineCol);
            }
        }
    }

    if (m_state->m_gameOver && m_state->m_winner != Winner_Drawn) {
        int start_i = m_state->win_start, end_i = m_state->win_end;
        ind_to_coord(start_i, start_x, start_y, m_state);
        ind_to_coord(end_i, end_x, end_y, m_state);
        Vector2 start_pos = cell_center(m_state->x_offset + start_x * cellSize,
            m_state->y_offset + start_y * cellSize, cellSize);
        Vector2 end_pos = cell_center(m_state->x_offset + end_x * cellSize,
            m_state->y_offset + end_y * cellSize, cellSize);
        DrawLineEx(start_pos, end_pos, 10, (Color) { 255, 255, 255, 190 });
    }
    // BeginShaderMode(m_state->confetti);
    // // DrawTexture(texture, 5, 5, RED);
    // DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), CLITERAL(Color){255, 0, 0, 0});
    // EndShaderMode();
}

void* ttt_pre_reload(ttt_state* m_state) {
    ttt_state* ptr = (ttt_state*)malloc(sizeof(*m_state));
    memcpy(ptr, m_state, sizeof(*m_state));
    int sz = m_state->m_size;
    ptr->m_state = (Turn*)malloc(sizeof(Turn*) * sz * sz);
    memcpy(ptr->m_state, m_state->m_state, sizeof(Turn*) * sz * sz);
    free(m_state->m_state);
    free(m_state);
    return ptr;
}

void ttt_post_reload(ttt_state* m_state, void* ptr) {
    m_state = (ttt_state*)malloc(sizeof(*m_state));
    memcpy(m_state, ptr, sizeof(*m_state));
    free(ptr);
}
