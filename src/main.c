#include "raylib.h"
#include <stdio.h>
#include <assert.h>

#define KDEBUG

#define __i64 long long

typedef signed char i8;
typedef unsigned char u8;
typedef short i16;
typedef unsigned short u16;
typedef int i32;
typedef unsigned u32;
typedef __i64 i64;
typedef unsigned __i64 u64;
typedef float f32;

#if !defined(_INC_STDLIB)
  #include <stdlib.h>
#endif

typedef size_t usize;

typedef Vector2 V2;

#define kassert assert

#define panic(...) do { fprintf(stderr,  __VA_ARGS__); exit(0); } while (0)
#define nonnull(this) ({ void* _ptr = (this); if (!_ptr) { panic("f:%s l:%d ERR: %s\n", __FILE__, __LINE__, "unwrap on a null value."); } _ptr; })
#define unwrap(this) ({ size_t _this = (this); if (!_this) panic("f:%s l:%d ERR: %s\n", __FILE__, __LINE__, "unwrap failed."); _this; })

#define WINDOW_HEIGHT 600 
#define WINDOW_WIDTH 800
#define FONT_SIZE_DEFAULT 20

#define KGRAY CLITERAL(Color){ 20, 20, 20, 255 }

typedef struct Context {
    Vector2 mouse_pos;
} Context;

typedef enum {
    UI_STATE_DEFAULT = 0,
    UI_STATE_ACTIVE,
    UI_STATE_HOVER,
} UIState;

// typedef struct StrSlice {
//     const char* ptr;
//     u64 len;
// } StrSlice;

// typedef struct Button {
//     StrSlice label;
// } Button;

// StrSlice strslice_from(const char* cstr) {
//     return (StrSlice){.ptr = cstr, .len = (u64)strlen(cstr)};
// }

typedef enum {
    COMPONENT_KIND_LAYOUT = 0,
    COMPONENT_KIND_RECT,
} UIComponentKind; 

// typedef struct UIComponent {
//     UIState state;
//     union variant { Button button; Layout layout; };
//     UIComponentKind kind;
//     u32 x;
//     u32 y;
//     u32 w;
//     u32 h;
// } UIComponent;

typedef enum {
    ORIENTATION_HORIZONTAL = 0,
    ORIENTATION_VERTICAL,
} LayoutOrientation;

typedef struct Layout {
    LayoutOrientation orientation;
    float padding;
    V2 position;
    V2 size;
} Layout;

#define v2_add(v1, v2) ((V2) { .x = (v1).x + (v2).x, .y = (v1).y + (v2).y, })

Vector2 layout_available_position(Layout* layout) {
    Vector2 padding = (Vector2) { .x = layout->padding, .y = layout->padding };
    Vector2 padded_position = v2_add(layout->size, padding);

    switch (layout->orientation) {
        case ORIENTATION_HORIZONTAL:
            padded_position.y = 0.0;
            break;
        case ORIENTATION_VERTICAL:
            padded_position.x = 0.0;
            break;
        default:
            panic("");
    }

    Vector2 shifted_position = v2_add(layout->position, padded_position);

    return shifted_position;
} 

// TODO: make typesafe
#define maxf(a, b) ((a) > (b) ? (a) : (b))

void layout_inflate(Layout* layout, V2 size) {
    switch (layout->orientation) {
        case ORIENTATION_HORIZONTAL:
            layout->size.x += size.x + layout->padding;
            layout->size.y = maxf(size.y, layout->padding);
            break;
        case ORIENTATION_VERTICAL:
            layout->size.x = maxf(size.x, layout->size.x);
            layout->size.y += size.y + layout->padding;
            break;
        defaut:
            panic("");
    }
}

#define LAYOUT_MAX_CAP 256

typedef struct UI {
    // TODO: dynamic
    Layout layouts[LAYOUT_MAX_CAP];
    usize layout_count;
} UI;

typedef struct UIRect {
    Color color;
    usize w;
    usize h;
} UIRect;

typedef struct AppState {
    UI ui;
    Context ctx;
} AppState;

void ui_layout_push(UI* ui, Layout layout) {
    kassert(ui->layout_count < LAYOUT_MAX_CAP);
    ui->layouts[ui->layout_count++] = layout;
}

Layout ui_layout_pop(UI* ui) {
    kassert(ui->layout_count > 0);
    return ui->layouts[ui->layout_count--];
}

Layout* ui_layout_last(UI* ui) {
    return ui->layout_count > 0 ? &ui->layouts[ui->layout_count - 1] : NULL;
}

void ui_begin(UI* ui, V2 position) {
    ui_layout_push(ui, (Layout) {
        .orientation = ORIENTATION_HORIZONTAL, 
        .padding = 0.0, 
        .position = position,
    });
}

void ui_layout_begin(UI* ui, Layout child) {
    Layout* parent = nonnull(ui_layout_last(ui));
    child.position = layout_available_position(parent);
    child.size = (V2){0};
    ui_layout_push(ui, child);
}

void ui_layout_end(UI* ui) {
    Layout child = ui_layout_pop(ui);
    Layout* parent = nonnull(ui_layout_last(ui));
    layout_inflate(parent, child.size);
}

void ui_end(UI* ui) { ui_layout_pop(ui); }

void ui_rect(UI* ui, UIRect rect) { 
    Layout* parent = nonnull(ui_layout_last(ui));
    Vector2 position = layout_available_position(parent);
    layout_inflate(parent, (Vector2) { .x = rect.w, .y = rect.h });
    DrawRectangle(position.x, position.y, rect.w, rect.h, rect.color);
}

#ifdef KDEBUG
bool is_debug_mode_ui = false;

static inline void toggle_debug_info(void) { is_debug_mode_ui = !is_debug_mode_ui; }

static inline void draw_debug_info(Context* ctx) {
    int mouseX = ctx->mouse_pos.x;
    int mouseY = ctx->mouse_pos.y;

    DrawText(TextFormat("mouse { x: %d y: %d }", mouseX, mouseY), 50,
             WINDOW_HEIGHT - 30, FONT_SIZE_DEFAULT, WHITE);

    DrawFPS(WINDOW_WIDTH - 100, WINDOW_HEIGHT - 30);
}
#endif  // KDEBUG

static inline void update_context(Context* ctx) {
    ctx->mouse_pos = GetMousePosition();
}

void app_update_state(AppState* state) {
    update_context(&state->ctx);
}

void app_render(AppState* state) {
    UI* ui = &state->ui;
    ClearBackground(KGRAY);

    ui_begin(ui, (V2){10, 10});
    ui_layout_begin(ui, (Layout){.orientation = ORIENTATION_HORIZONTAL, .padding = 5.0});
    {
        ui_rect(ui, (UIRect){.color = RED, .w = 100, .h = 100});
        ui_rect(ui, (UIRect){.color = GREEN, .w = 100, .h = 100});
        ui_rect(ui, (UIRect){.color = WHITE, .w = 75, .h = 100});
    }
    ui_layout_end(ui);
    ui_end(ui);

#ifdef KDEBUG
    if (IsKeyPressed(KEY_D)) { toggle_debug_info(); }; 
    if (is_debug_mode_ui) { draw_debug_info(&state->ctx); };
#endif  // KDEBUG
}

int main(void) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "KACHAN");
    SetTargetFPS(60);
    SetTraceLogLevel(LOG_ALL);

    AppState state = {0};

    while (!WindowShouldClose()) {
        app_update_state(&state);
        BeginDrawing();
        app_render(&state);
        EndDrawing();
    }
    
    CloseWindow();
}