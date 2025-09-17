#include "raylib.h"
#include <stdio.h>
#include <assert.h>

#define KDEBUG
#define KRENDERER_RAYLIB

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

#ifdef KRENDERER_RAYLIB
typedef Vector2 V2;
#else
  #error "Unimplemented renderer"
#endif  // KRENDERER_RAYLIB

#define kassert assert

#define panic(...) do { fprintf(stderr,  __VA_ARGS__); exit(0); } while (0)
#define nonnull(this) __extension__ ({ void* _ptr = (this); if (!_ptr) { panic("f:%s l:%d ERR: %s\n", __FILE__, __LINE__, "unwrap on a null value."); } _ptr; })
#define unwrap(this) __extension__ ({ size_t _this = (this); if (!_this) panic("f:%s l:%d ERR: %s\n", __FILE__, __LINE__, "unwrap failed."); _this; })

#define v2_add(v1, v2) ((V2) { .x = (v1).x + (v2).x, .y = (v1).y + (v2).y, })

#define max(a, b) __extension__ ({ __typeof__ (a) _a = (a);  __typeof__ (b) _b = (b); _a > _b ? _a : _b; })

#define WINDOW_HEIGHT 600 
#define WINDOW_WIDTH 800
#define FONT_SIZE_DEFAULT 20

#define KGRAY CLITERAL(Color){ 20, 20, 20, 255 }

typedef struct Context {
    Vector2 mouse_pos;
} Context;

typedef enum {
    RENDER_COMMAND_NONE = 0,
    RENDER_COMMAND_RECT,
} RenderCommandKind;

typedef struct RenderCommandRect {
    V2 pos;
    u32 w;
    u32 h;
    Color color;
} RenderCommandRect;

typedef struct RenderCommand {
    union {
        RenderCommandRect rect;
    } variant;
    RenderCommandKind kind;
} RenderCommand;

RenderCommand render_rect(V2 pos, u32 w, u32 h, Color color) {
    return (RenderCommand) {
        .variant = {
            .rect = {.pos = pos, .w = w, .h = h, .color = color},
        }, 
        .kind = RENDER_COMMAND_RECT
    };
}

typedef struct RenderCommands {
    RenderCommand* items;
    usize len;
} RenderCommands;

static inline RenderCommands render_commands_new(RenderCommand* items, usize len) {
    return (RenderCommands){.items = items, .len = len};
}

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
            panic("unreachable");
    }

    Vector2 shifted_position = v2_add(layout->position, padded_position);

    return shifted_position;
} 

void layout_inflate(Layout* layout, V2 size) {
    switch (layout->orientation) {
        case ORIENTATION_HORIZONTAL:
            layout->size.x += size.x + layout->padding;
            layout->size.y = max(size.y, layout->padding);
            break;
        case ORIENTATION_VERTICAL:
            layout->size.x = max(size.x, layout->size.x);
            layout->size.y += size.y + layout->padding;
            break;
        default:
            panic("unreachable");
    }
}

typedef struct UIRect {
    Color color;
    usize w;
    usize h;
} UIRect;

#define LAYOUT_MAX_CAP 256
#define RENDER_COMMANDS_MAX_CAP 256

typedef struct UI {
    // TODO: dynamic
    Layout layouts[LAYOUT_MAX_CAP];
    RenderCommand render_commands[RENDER_COMMANDS_MAX_CAP];
    usize render_commands_count;
    usize layout_count;
} UI;

void ui_layout_push(UI* ui, Layout layout) {
    kassert(ui->layout_count < LAYOUT_MAX_CAP);
    ui->layouts[ui->layout_count++] = layout;
}

Layout ui_layout_pop(UI* ui) {
    kassert(ui->layout_count > 0);
    return ui->layouts[ui->layout_count--];
}

Layout* ui_layout_parent(UI* ui) {
    return ui->layout_count > 0 ? &ui->layouts[ui->layout_count - 1] : NULL;
}

void ui_begin(UI* ui, V2 position) {
    ui->render_commands_count = 0;

    ui_layout_push(ui, (Layout) {
        .orientation = ORIENTATION_HORIZONTAL, 
        .padding = 0.0, 
        .position = position,
    });
}

void ui_layout_begin(UI* ui, Layout child) {
    Layout* parent = nonnull(ui_layout_parent(ui));
    child.position = layout_available_position(parent);
    child.size = (V2){0};
    ui_layout_push(ui, child);
}

void ui_layout_end(UI* ui) {
    Layout child = ui_layout_pop(ui);
    Layout* parent = nonnull(ui_layout_parent(ui));
    layout_inflate(parent, child.size);
}

void ui_end(UI* ui) { ui_layout_pop(ui); }

void ui_render_widget(UI* ui, RenderCommand render_command) {
    ui->render_commands[ui->render_commands_count++] = render_command;
}

void ui_rect(UI* ui, UIRect rect) { 
    Layout* parent = nonnull(ui_layout_parent(ui));
    Vector2 position = layout_available_position(parent);
    layout_inflate(parent, (Vector2) { .x = rect.w, .y = rect.h });

    // DrawRectangle(position.x, position.y, rect.w, rect.h, rect.color);
    ui_render_widget(ui, render_rect(position, rect.w, rect.h, rect.color));
}

RenderCommands ui_render(UI* ui) {
    ui_begin(ui, (V2){10, 10});
    {
        ui_layout_begin(ui, (Layout){.orientation = ORIENTATION_VERTICAL, .padding = 10.0});
        {
            ui_rect(ui, (UIRect){.color = RED, .w = 100, .h = 100});
            ui_rect(ui, (UIRect){.color = GREEN, .w = 100, .h = 100});
            ui_rect(ui, (UIRect){.color = WHITE, .w = 75, .h = 100});
        }
        ui_layout_end(ui);
    }
    ui_end(ui);

    return render_commands_new(ui->render_commands, ui->render_commands_count); 
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

typedef struct AppState {
    UI ui;
    Context ctx;
} AppState;

static inline void app_update_state(AppState* state) {
    state->ctx.mouse_pos = GetMousePosition();
}

void app_render(Context* ctx, RenderCommands* render_commands) {
    ClearBackground(KGRAY);

    for (u32 i = 0; i < render_commands->len; ++i) {
        RenderCommand command = render_commands->items[i]; 

        switch (command.kind) {
            case RENDER_COMMAND_RECT: {
                RenderCommandRect rect = command.variant.rect;
                DrawRectangle(rect.pos.x, rect.pos.y, rect.w, rect.h, rect.color);
                break;
            }
            default:
                break;
        }
    }

#ifdef KDEBUG
    if (IsKeyPressed(KEY_D)) { toggle_debug_info(); }; 
    if (is_debug_mode_ui) { draw_debug_info(ctx); };
#endif  // KDEBUG
}

// TODO: add fix naming and remove unused code/comments
// TODO: implement widget id hashing
// TODO: implement is_hover(id)
// TODO: implement a ui arena allocator for dynamic buffers
// TODO: make id_hashing static in the long run
int main(void) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "KACHAN");
    SetTargetFPS(60);
    SetTraceLogLevel(LOG_ALL);

    AppState state = {0};

    while (!WindowShouldClose()) {
        app_update_state(&state);
        RenderCommands commands = ui_render(&state.ui);
        BeginDrawing();
        app_render(&state.ctx, &commands);
        EndDrawing();
    }
    
    CloseWindow();
}