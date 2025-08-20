#include "raylib.h"

#define KDEBUG

#define panic(...) do { fprintf(stderr,  __VA_ARGS__); exit(0); } while (0)
#define nonnull(this) ({ void* _ptr = (this); if (!_ptr) panic("f:%s l:%d ERR: %s\n", __FILE__, __LINE__, "unwrap on a null value."); _ptr; })

#define WINDOW_HEIGHT 600 
#define WINDOW_WIDTH 800
#define FONT_SIZE_DEFAULT 20

typedef struct Context {
    Vector2 mouse_pos;
} Context;

#ifdef KDEBUG
bool is_debug_mode_ui = false;

static inline void toggle_debug_ui(void) { is_debug_mode_ui = !is_debug_mode_ui; }

static inline void draw_debug_info(Context* ctx) {
    int mouseX = ctx->mouse_pos.x;
    int mouseY = ctx->mouse_pos.y;

    DrawText(TextFormat("mouse { x: %d y: %d }", mouseX, mouseY), 50,
             WINDOW_HEIGHT - 30, FONT_SIZE_DEFAULT, WHITE);
}
#endif  // KDEBUG

void update_context(Context* ctx) {
    ctx->mouse_pos = GetMousePosition();
}

void update_draw_frame(Context* ctx) {
    update_context(ctx);

    ClearBackground(BLACK);
    #ifdef KDEBUG
    if (IsKeyPressed(KEY_D)) { toggle_debug_ui(); }; 
    if (is_debug_mode_ui) { draw_debug_info(ctx); };
    #endif // KDEBUG
}


int main(void) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "KACHAN");
    SetTargetFPS(60);
    SetTraceLogLevel(LOG_ALL);

    Context ctx = {0};

    while (!WindowShouldClose()) {
        BeginDrawing();
        update_draw_frame(&ctx);
        EndDrawing();
    }
    
    CloseWindow();
}