#include <raylib-ext.hpp>
#include <string>

struct viewport_t
{
    float left, right;
    float bottom, top;
};

struct context_t
{
    Vector2 screen_size;
    viewport_t viewport;
};

Vector2
screen_to_local(context_t *ctx, Vector2 point)
{
    Vector2 d = {
        ctx->viewport.right - ctx->viewport.left,
        ctx->viewport.top - ctx->viewport.bottom
    };
    return Vector2 {
        (point.x / ctx->screen_size.x) * d.x + ctx->viewport.left,
        (point.y / ctx->screen_size.y) * d.y + ctx->viewport.bottom,
    };
}

Rectangle
fix_rect(Rectangle rect)
{
    Rectangle fixed = rect;
    if (fixed.width < 0)
    {
        fixed.x += fixed.width;
        fixed.width *= -1;
    }
    if (fixed.height < 0)
    {
        fixed.y += fixed.height;
        fixed.height *= -1;
    }
    return fixed;
}

void
set_viewport(context_t *context, Rectangle rect)
{
    Vector2 p0 = screen_to_local(context, Vector2 { rect.x, rect.y });
    Vector2 p1 = screen_to_local(
        context, Vector2 {
            rect.x + rect.width,
            rect.y + rect.height
        }
    );
    context->viewport = viewport_t {
        p0.x,
        p1.x,
        p0.y,
        p1.y,
    };
}

int
main(void)
{
    const int screen_width = 1280;
    const int screen_height = 800;
    const float aspect = screen_width * 1.0 / screen_height;

    InitWindow(screen_width, screen_height, "Creative Coding: Mandelbrot Set");
    SetTargetFPS(60);

    context_t context = {
        Vector2 { screen_width, screen_height },
        viewport_t { -2, 0.5, -1.12, 1.12 },
    };
    set_viewport(&context, { 0, 0, screen_width, screen_height });

    Rectangle selected_rect = { 0, 0, 0, 0 };
    bool selecting = false;

    RenderTexture2D target = LoadRenderTexture(screen_width, screen_height);
    Shader shader = LoadShader(0, "Assets/shaders/mandelbrot.frag");
    SetShaderValue(
        shader,
        GetShaderLocation(shader, "resolution"),
        (float[2]) {float(screen_width), float(screen_height)},
        SHADER_UNIFORM_VEC2
    );
    int viewport_loc = GetShaderLocation(shader, "viewport");
    SetShaderValue(shader, viewport_loc, &context.viewport, SHADER_UNIFORM_VEC4);

    while (!WindowShouldClose())
    {
        float deltatime = GetFrameTime();
        char title[128];
        snprintf(
            title, sizeof(title),
            "Creative Coding: Mandelbrot Set [fps = %f]",
            1 / deltatime
        );
        SetWindowTitle(title);

        BeginTextureMode(target);
            ClearBackground(BLACK);
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), BLACK);
        EndTextureMode();

        BeginDrawing();
        {
            ClearBackground(BLACK);
            BeginShaderMode(shader);
                DrawTextureEx(target.texture, (Vector2){ 0.0f, 0.0f }, 0.0f, 1.0f, WHITE);
            EndShaderMode();

            if (selecting)
            {
                Rectangle fixed = fix_rect(selected_rect);
                DrawRectangleRec(fixed, { 255, 255, 255, 50 });
                DrawRectangleLinesEx(fixed, 1, WHITE);
            }
        }
        EndDrawing();

        if (IsKeyPressed(KEY_SPACE))
        {
            context.viewport = { -2, 0.5, -1.12, 1.12 };
            set_viewport(&context, { 0, 0, screen_width, screen_height });
            SetShaderValue(shader, viewport_loc, &context.viewport, SHADER_UNIFORM_VEC4);
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            selected_rect.x = float(GetMouseX());
            selected_rect.y = float(GetMouseY());
            selecting = true;

        }

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
        {
            selected_rect.width =  float(GetMouseX()) - selected_rect.x;
            selected_rect.height = float(GetMouseY()) - selected_rect.y;
            selected_rect.height = copysign(selected_rect.width / aspect, selected_rect.height);
        }

        if (selecting && IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        {
            set_viewport(&context, fix_rect(selected_rect));
            SetShaderValue(shader, viewport_loc, &context.viewport, SHADER_UNIFORM_VEC4);
        }

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        {
            selected_rect = { 0, 0, 0, 0 };
            selecting = false;
        }
    }

    UnloadShader(shader);
    UnloadRenderTexture(target);
    CloseWindow();

    return 0;
}
