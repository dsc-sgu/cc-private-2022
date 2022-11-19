#include <raylib-ext.hpp>
#include <string>

struct v2d
{
    double x, y;
};

struct viewport_t
{
    double left, right;
    double bottom, top;
};

struct context_t
{
    Vector2 screen_size;
    viewport_t viewport;
};

v2d
screen_to_local(context_t *ctx, v2d point)
{
    v2d d = {
        ctx->viewport.right - ctx->viewport.left,
        ctx->viewport.top - ctx->viewport.bottom
    };
    return v2d {
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
    v2d p0 = screen_to_local(context, v2d { rect.x, rect.y });
    v2d p1 = screen_to_local(
        context, v2d {
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

Vector2 double_to_ds(double d)
{
     float x = (float) d;
     float y = d - (double) x;
     return Vector2 { x, y };
}

int
main(void)
{
    const int screen_width = 1280;
    const int screen_height = 800;
    /* const int screen_width = 1440; */
    /* const int screen_height = 900; */
    const double aspect = screen_width * 1.0 / screen_height;

    InitWindow(screen_width, screen_height, "Creative Coding: Mandelbrot Set");
    /* ToggleFullscreen(); */

    context_t context = {
        Vector2 { screen_width, screen_height },
        viewport_t { -2, 0.5, -1.12, 1.12 },
    };
    set_viewport(&context, { 0, 0, screen_width, screen_height });

    Rectangle selected_rect = { 0, 0, 0, 0 };
    bool selecting = false;

    // Mandelbrot shader
    RenderTexture2D target = LoadRenderTexture(screen_width, screen_height);
    Shader shader = LoadShader(0, "Assets/shaders/mandelbrot.frag");
    SetShaderValue(
        shader,
        GetShaderLocation(shader, "resolution"),
        (float[2]) {float(screen_width), float(screen_height)},
        SHADER_UNIFORM_VEC2
    );
    int viewport_loc = GetShaderLocation(shader, "viewport");
    viewport_t v = context.viewport;
    Vector2 v1 = double_to_ds(v.left);
    Vector2 v2 = double_to_ds(v.right);
    Vector2 v3 = double_to_ds(v.top);
    Vector2 v4 = double_to_ds(v.bottom);
    SetShaderValue(shader, GetShaderLocation(shader, "l"), (float*)(&v1), SHADER_UNIFORM_VEC2);
    SetShaderValue(shader, GetShaderLocation(shader, "r"), (float*)(&v2), SHADER_UNIFORM_VEC2);
    SetShaderValue(shader, GetShaderLocation(shader, "t"), (float*)(&v3), SHADER_UNIFORM_VEC2);
    SetShaderValue(shader, GetShaderLocation(shader, "b"), (float*)(&v4), SHADER_UNIFORM_VEC2);

    float time = 0.0f;
    int time_loc = GetShaderLocation(shader, "time");
    SetShaderValue(shader, time_loc, &time, SHADER_UNIFORM_FLOAT);

    while (!WindowShouldClose())
    {
        float deltatime = GetFrameTime();
        time += deltatime;
        SetShaderValue(shader, time_loc, &time, SHADER_UNIFORM_FLOAT);

        char title[128];
        snprintf(
            title, sizeof(title),
            "Creative Coding: Mandelbrot Set [fps = %f]",
            1 / deltatime
        );
        SetWindowTitle(title);

        BeginTextureMode(target);
            BeginShaderMode(shader);
                DrawTextureEx(target.texture, (Vector2){ 0.0f, 0.0f }, 0.0f, 1.0f, WHITE);
            EndShaderMode();
        EndTextureMode();

        BeginDrawing();
        {
            ClearBackground(BLACK);

            DrawTextureRec(
                target.texture,
                Rectangle {0, 0, float(target.texture.width), float(-target.texture.height)},
                Vector2 {0, 0},
                WHITE
            );

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
            viewport_t v = context.viewport;
            Vector2 v1 = double_to_ds(v.left);
            Vector2 v2 = double_to_ds(v.right);
            Vector2 v3 = double_to_ds(v.top);
            Vector2 v4 = double_to_ds(v.bottom);
            SetShaderValue(shader, GetShaderLocation(shader, "l"), (float*)(&v1), SHADER_UNIFORM_VEC2);
            SetShaderValue(shader, GetShaderLocation(shader, "r"), (float*)(&v2), SHADER_UNIFORM_VEC2);
            SetShaderValue(shader, GetShaderLocation(shader, "t"), (float*)(&v3), SHADER_UNIFORM_VEC2);
            SetShaderValue(shader, GetShaderLocation(shader, "b"), (float*)(&v4), SHADER_UNIFORM_VEC2);
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
            viewport_t v = context.viewport;
            Vector2 v1 = double_to_ds(v.left);
            Vector2 v2 = double_to_ds(v.right);
            Vector2 v3 = double_to_ds(v.top);
            Vector2 v4 = double_to_ds(v.bottom);
            SetShaderValue(shader, GetShaderLocation(shader, "l"), (float*)(&v1), SHADER_UNIFORM_VEC2);
            SetShaderValue(shader, GetShaderLocation(shader, "r"), (float*)(&v2), SHADER_UNIFORM_VEC2);
            SetShaderValue(shader, GetShaderLocation(shader, "t"), (float*)(&v3), SHADER_UNIFORM_VEC2);
            SetShaderValue(shader, GetShaderLocation(shader, "b"), (float*)(&v4), SHADER_UNIFORM_VEC2);
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
