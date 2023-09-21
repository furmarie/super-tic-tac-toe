#include <raylib.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "hot_reload.h"

int main(void) {
    if (!reload_libplug()) {
        return 1;
    }
    int width = 800, height = 800;
    InitWindow(width, height, "TTT");
    SetTargetFPS(30);

    s_ttt_init(3, width, height);

    while (!WindowShouldClose()) {
        BeginDrawing();
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_R)) {
            void* state = s_ttt_pre_reload();
            if (!reload_libplug()) {
                return 1;
            }
            s_ttt_post_reload(state);
        }
        else if (IsKeyPressed(KEY_R)) {
           s_ttt_reset();
        }
        s_ttt_draw(GetFrameTime());
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
