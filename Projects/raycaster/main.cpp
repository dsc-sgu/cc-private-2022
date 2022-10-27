#include <raylib-ext.hpp>
#include <algorithm>
#include <vector>
#include <iostream>

#define RUN_RAYCASTER
// #define RAYCAST_TEXTURES

const int screenWidth = 1440;
const int screenHeight = 900;

const int board_w = 8;
const int board_h = 8;

const int worldSize = 800;
const int cell_size = worldSize / board_w;

int board[board_w][board_h] = {
    { 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 2, 0, 0, 0, 1, 1 },
    { 1, 2, 2, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 1, 0, 0, 1 },
    { 1, 0, 1, 0, 0, 0, 0, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1 },
};
Color wall_colors[] = {
    BLANK,
    RED,
    BLUE,
};

struct player_t
{
    Vector2 pos;
    float rotation;
    float speed;
};

#ifdef RUN_RAYCASTER
struct hit_t {
    Vector2 pos;
    struct { int x, y; } cell_pos;
    bool is_horizontal;
    float angle;
};
#endif

struct RenderConfig
{
#ifdef RUN_RAYCASTER
    std::vector<hit_t> hits;
    float fov;
    int rays_count;
    float delta_angle;
    float rect_w;
#endif
};


bool
check_collision(Vector2 position, float radius)
{
    for (float angle = -PI; angle < PI; angle += PI / 4)
    {
        Vector2 check = position + Vector2Rotate({ radius, 0 }, angle);
        int cell_x = check.x / cell_size;
        int cell_y = check.y / cell_size;
        if (board[cell_x][cell_y] != 0)
            return true;
    }
    return false;
}

void
DrawTopDownView(const player_t &player, const RenderConfig &config)
{
    for (int row = 0; row < board_h; ++row) {
        for (int col = 0; col < board_w; ++col) {
            if (board[col][row] != 0) {
                DrawRectangle(col * cell_size, row * cell_size,
                    cell_size, cell_size, BLACK);
            }
            else
            {
                DrawRectangle(col * cell_size, row * cell_size,
                    cell_size, cell_size, WHITE);
            }
        }
    }

    for (int x = cell_size; x < worldSize; x += cell_size) {
        DrawLine(x, 0, x, worldSize, GRAY);
    }
    for (int y = cell_size; y < worldSize; y += cell_size) {
        DrawLine(0, y, worldSize, y, GRAY);
    }

    DrawCircleV(player.pos, 14, RED);
#ifdef RUN_RAYCASTER
    for (const hit_t &hit : config.hits)
        DrawLineEx(player.pos, hit.pos, 2, BLUE);
#else
    DrawLineEx(player.pos, player.pos + Vector2Rotate({ 1,0 }, player.rotation) * 25, 5, BLUE);
#endif


}

#ifdef RUN_RAYCASTER
Image images[] = {
    LoadImage("./Assets/textures/TECH_1A.png"), // NULL
    LoadImage("./Assets/textures/walltile.png"),
    LoadImage("./Assets/textures/walltile2.png"),
};

inline bool
correct_cell(int x, int y)
{
    return (x >= 0 && x < board_w) && (y >= 0 && y < board_h);
}

inline float
fix_angle(float angle)
{
    while (angle > PI)  angle -= 2 * PI;
    while (angle < -PI) angle += 2 * PI;
    return angle;
}

hit_t cast_ray(Vector2 pos, float dir)
{
    dir = fix_angle(dir);

    int cell_x = pos.x / cell_size;
    int cell_y = pos.y / cell_size;

    hit_t hit_data_v, hit_data_h;

    // Vertical hit
    for (int k = 0; ; ++k) {
        int shift;
        int k_dir;
        if (dir > -PI / 2 && dir < PI / 2) {
            shift = 1;
            k_dir = 1;
        }
        else {
            shift = 0;
            k_dir = -1;
        }

        float dx = (cell_x + shift + k * k_dir) * cell_size - pos.x;
        float dy = dx * tan(dir);
        Vector2 d = { dx, dy };
        Vector2 hit = d + pos;

        int cell_hit_x = int(hit.x / cell_size) + shift - 1;
        int cell_hit_y = int(hit.y / cell_size);

        hit_data_v.pos = hit;
        hit_data_v.cell_pos = { cell_hit_x, cell_hit_y };
        hit_data_v.is_horizontal = false;
        hit_data_v.angle = dir;

        if (!correct_cell(cell_hit_x, cell_hit_y))
            break;
        if (board[cell_hit_x][cell_hit_y] != 0)
            break;
    }

    // Horizontal hit
    for (int k = 0; ; ++k) {
        int shift;
        int k_dir;
        if (dir > -PI && dir < 0) {
            shift = 0;
            k_dir = -1;
        }
        else {
            shift = 1;
            k_dir = 1;
        }

        float dy = (cell_y + shift + k * k_dir) * cell_size - pos.y;
        float dx = dy / tan(dir);
        Vector2 d = { dx, dy };
        Vector2 hit = d + pos;

        int cell_hit_x = int(hit.x / cell_size);
        int cell_hit_y = int(hit.y / cell_size) + shift - 1;

        hit_data_h.pos = hit;
        hit_data_h.cell_pos = { cell_hit_x, cell_hit_y };
        hit_data_h.is_horizontal = true;
        hit_data_h.angle = dir;

        if (!correct_cell(cell_hit_x, cell_hit_y))
            break;
        if (board[cell_hit_x][cell_hit_y] != 0)
            break;
    }

    if (Vector2Length(hit_data_h.pos - pos) < Vector2Length(hit_data_v.pos - pos)) {
        return hit_data_h;
    }
    else {
        return hit_data_v;
    }
}

void
DrawRaycastView(const player_t &player, const RenderConfig &config)
{
    float rect_x = 0;
    for (const hit_t &hit : config.hits)
    {
        Vector2 hit_delta = hit.pos - player.pos;
        float dist = hit_delta.x * cos(player.rotation) +
            hit_delta.y * sin(player.rotation);

        float rect_h = (cell_size * screenHeight) / dist;
        float rect_y = (screenHeight - rect_h) / 2;

        int image_idx = board[hit.cell_pos.x][hit.cell_pos.y];
#ifdef RAYCAST_TEXTURES
        Vector2 pos_in_cell = {
            hit.pos.x - hit.cell_pos.x * cell_size,
            hit.pos.y - hit.cell_pos.y * cell_size,
        };
        Image cell_image = images[image_idx];
        Vector2 column = pos_in_cell / cell_size * cell_image.width;
        int col = column.y;
        if (hit.is_horizontal)
            col = column.x;

        float pix_h = rect_h / cell_image.height;

        for (int i = 0; i < cell_image.height; ++i)
        {
            Color* color_data = (Color*)cell_image.data;
            Color pixel = color_data[i * cell_image.width + col];

            DrawRectangle(
                rect_x, rect_y + pix_h * i,
                config.rect_w + 1, pix_h + 1, pixel
            );
        }
#else
        Color wall_color = wall_colors[image_idx];
        if (hit.is_horizontal)
            wall_color *= 0.8f;

        DrawRectangle(
            rect_x, rect_y,
            config.rect_w + 1, rect_h + 1, wall_color
        );
#endif

        rect_x += config.rect_w;
    }
}
#endif

int main()
{
#ifdef RUN_RAYCASTER
    InitWindow(screenWidth, screenHeight, "GDSC: Creative Coding");
#else
    InitWindow(screenWidth, screenHeight, "GDSC: Creative Coding");
#endif
    SetTargetFPS(60);
    ToggleFullscreen();

    player_t player;
    player.pos = { worldSize / 2, worldSize / 2 };
    player.speed = 100;
    player.rotation = 0;

    RenderTexture2D view = LoadRenderTexture(worldSize, worldSize);

#ifdef RUN_RAYCASTER
    RenderConfig config;
    config.fov = 85 * DEG2RAD;
    config.rays_count = 1440;
    config.delta_angle = config.fov / config.rays_count;
    config.rect_w = (screenWidth / config.fov) * config.delta_angle;
#endif

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        Vector2 move = { 0, 0 };
#ifdef RUN_RAYCASTER
        DisableCursor();
        float delta = GetMouseDelta().x;
        player.rotation += delta * dt * 0.1;
        Vector2 dir = Vector2Rotate({ 1, 0 }, player.rotation);
        Vector2 forward = dir * (player.speed * dt);
        Vector2 right = Vector2Rotate(forward, PI / 2);
        if (IsKeyDown(KEY_W))
            move = forward;
        if (IsKeyDown(KEY_S))
            move = -forward;
        if (IsKeyDown(KEY_A))
            move = -right;
        if (IsKeyDown(KEY_D))
            move = right;
#else
        if (IsKeyDown(KEY_W))
            move.y -= player.speed * dt;
        if (IsKeyDown(KEY_S))
            move.y += player.speed * dt;
        if (IsKeyDown(KEY_A))
            move.x -= player.speed * dt;
        if (IsKeyDown(KEY_D))
            move.x += player.speed * dt;

        Vector2 mp = {
            GetMouseX() - player.pos.x,
            GetMouseY() - player.pos.y
        };
        player.rotation = Vector2Angle({ 1, 0 }, mp);
#endif
        player.pos += move;
        if (check_collision(player.pos, 15))
            player.pos -= move;

        config.hits.clear();
        for (float angle = -config.fov / 2; angle < config.fov / 2; angle += config.delta_angle) {
            hit_t hit = cast_ray(player.pos, player.rotation + angle);
            config.hits.push_back(hit);
        }

        BeginTextureMode(view);
        {
            DrawTopDownView(player, config);
        }
        EndTextureMode();

        BeginDrawing();
        {
            ClearBackground(SKYBLUE);
            DrawRectangle(0, screenHeight / 2, screenWidth, screenHeight, BEIGE);
#ifdef RUN_RAYCASTER
            DrawRaycastView(player, config);
#endif
            float map_scale = 0.4;
            float map_size = map_scale * worldSize;
            DrawTexturePro(
                view.texture,
                Rectangle { 0, 0, worldSize, worldSize },
                Rectangle { 0, screenHeight - map_size, map_size, map_size },
                Vector2 { 0, 0 },
                0, WHITE
            );
        }
        EndDrawing();
    }
    CloseWindow();

    return 0;
}
