#include <raylib-ext.hpp>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cassert>
#include <cmath>

#define MARKER_FADEOUT_BASE (0.05)
#define ANT_MARKER_BASE (0.2)
#define ANT_WANDER_BASE (0.1)
#define ANT_ATTRACTION_BASE (0)
#define EPS (1e-5)

const int WindowsHeight = 600;
const int WindowsWidth = 800;

const int cell_radius = 2;
const int cell_fresh_period; // можно double для более точной настройки

struct cell_t
{
    int x;
    int y;
    
    uint8_t home_marker;
    uint8_t food_marker;
    
    // NODE(ForestGump): с помощью этого возможно удасться решить проблему
    unsigned fresh; // Свежесть
    unsigned order; // Порядок
    double distance_to_home_market; // Дистонция до дома/муровейника
    double distance_to_food_market; // Дистанция до метки с едой
};

enum ant_state_t
{
    GOING_HOME,
    SEARCHING_FOOD,
};

struct ant_t
{
    Vector2 position;
    Vector2 direction;

    ant_state_t state;

    float speed;
    float dir_cooldown;
    float marker_cooldown;
    float attraction_cooldown;

    // NODE(ForestGump):
    // Количество меток, оставленных муравьём 
    // Если муравей пришёл без еды, то метки удаляем, а счётчик обнуляем
    int cell_count;
};

struct food_source_t
{
    int charges;
    
    float radius;

    Vector2 position;
};

struct colony_t
{
    float radius;
    
    Vector2 position;

    uint32_t size;
};

typedef std::vector<std::vector<cell_t>> field_t;

void print_cell(cell_t &cell)
{
    std::cout << "Home: " << (int) cell.home_marker << "; Food: " << (int) cell.food_marker << std::endl << "; Freshness: " << (int)cell.fresh << "; Order: " << (int)cell.order;
}

float calculate_attractiveness(Vector2 point, ant_state_t state, field_t &field)
{
    Vector2 cells;
    
    cells.x = WindowsWidth / cell_radius;
    cells.y = WindowsHeight / cell_radius;

    int row = int(point.x);
    int col = int(point.y);

    int square_half = 1;

    std::vector<cell_t *> neighbors;

    for (int i = -square_half; i <= square_half; ++i)
    {
        for (int j = -square_half; j <= square_half; ++j)
        {
            int x = i + row;
            int y = j + col;
            
            if (x < 0 || x >= (int) cells.x || y < 0 || y >= (int) cells.y) continue;

            neighbors.push_back(&field[x][y]);
        }
    }

    std::pair<float, float> attractiveness;

    for (cell_t *neighbor : neighbors)
    {
        attractiveness.first += (float) neighbor->home_marker;
        attractiveness.second += (float) neighbor->food_marker;
    }
    
    if (neighbors.size() == 0) return 0;
    
    switch (state)
    {
        case GOING_HOME:
        {
            return attractiveness.first / float(neighbors.size());
        }
        case SEARCHING_FOOD:
        {
            return attractiveness.second / float(neighbors.size());
        }
        default:
        {
            // NODE(ForestGump):
            // Тут не нужен break или что-то типа того?
            // Или assert может выполнять туже функцию?
            assert(false);
        }
    }
}

std::vector<Vector2> attractive_points(3);

cell_t *find_attractive_cell2(ant_t &ant, field_t &field)
{
    Vector2 cells;
    
    cells.x = WindowsWidth / cell_radius;
    cells.y = WindowsHeight / cell_radius;

    Vector2 ant_screen = ant.position;
    
    int row = (int) (ant_screen.x / cell_radius);
    int col = (int) (ant_screen.y / cell_radius);

    row = std::clamp(row, 0, int(cells.x) - 1);
    col = std::clamp(col, 0, int(cells.y) - 1);

    float scale = cell_radius * 20;

    Vector2 center = ant.position + ant.direction * scale;
    Vector2 left = ant.position + Vector2Rotate(ant.direction,  45 * DEG2RAD) * scale;
    Vector2 right = ant.position + Vector2Rotate(ant.direction, -45 * DEG2RAD) * scale;

    float center_attr = calculate_attractiveness(center, ant.state, field);
    float left_attr = calculate_attractiveness(left, ant.state, field);
    float right_attr = calculate_attractiveness(right, ant.state, field);

    if (std::abs(center_attr) < EPS && std::abs(left_attr) < EPS && std::abs(right_attr) < EPS)
    {
        return nullptr;
    }
    else if (center_attr >= std::max(left_attr, right_attr))
    {
        return &field[int(center.x) / cell_radius][int(center.y) / cell_radius];
    }
    else if (left_attr > right_attr)
    {
        return &field[int(left.x) / cell_radius][int(left.y) / cell_radius];
    }
    else
    {
        return &field[int(right.x) / cell_radius][int(right.y) / cell_radius];
    }
}

cell_t *find_attractive_cell(ant_t &ant, field_t &field)
{
    Vector2 cells;

    cells.x = WindowsWidth / cell_radius;
    cells.y = WindowsHeight / cell_radius;

    Vector2 ant_screen = ant.position;

    int row = (int) (ant_screen.x / cell_radius);
    int col = (int) (ant_screen.y / cell_radius);

    row = std::clamp(row, 0, int(cells.x) - 1);
    col = std::clamp(col, 0, int(cells.y) - 1);

    int square_half = 5;
    
    std::vector<cell_t *> neighbors;

    for (int i = -square_half; i <= square_half; ++i)
    {
        for (int j = -square_half; j <= square_half; ++j)
        {
            int x = i + row;
            int y = j + col;
            
            if (x < 0 || x >= (int) cells.x || y < 0 || y >= (int) cells.y) continue;
            
            neighbors.push_back(&field[x][y]);
        }
    }

    // NODE(ForestGump): Что это, почему 1е9?
    float min_val = 1e9;
    
    cell_t *cell = nullptr;

    for (cell_t *neighbor : neighbors)
    {
        float a = std::abs(row - neighbor->x);
        float b = std::abs(col - neighbor->y);

        float distance = sqrt(a * a + b * b);
        float value;

        switch (ant.state)
        {
            case GOING_HOME:
            {
                value = neighbor->home_marker / distance;

                if (neighbor->home_marker > 0 && value < min_val)
                {
                    cell = neighbor;
                    min_val = value;
                }

                break;
            }
            case SEARCHING_FOOD:
            {
                value = neighbor->food_marker / distance;

                if (neighbor->food_marker > 0 && value < min_val)
                {
                    cell = neighbor;
                    min_val = value;
                }

                break;
            }
        }
    }

    return cell;
}

void ant_update(ant_t &ant, field_t &field, std::vector<food_source_t> &sources, colony_t &colony, float dt)
{
    ant.dir_cooldown -= dt;
    ant.marker_cooldown -= dt;
    ant.attraction_cooldown -= dt;

    Vector2 cells;
    
    cells.x = WindowsWidth / cell_radius;
    cells.y = WindowsHeight / cell_radius;

    Vector2 ant_screen = ant.position;

    int row = (int) (ant_screen.x / cell_radius);
    int col = (int) (ant_screen.y / cell_radius);

    row = std::clamp(row, 0, int(cells.x) - 1);
    col = std::clamp(col, 0, int(cells.y) - 1);

    if (ant.dir_cooldown <= 0)
    {
        float rotation = (std::rand() % 20 - 10) * DEG2RAD;

        ant.dir_cooldown = ANT_WANDER_BASE;
        ant.direction = Vector2Rotate(ant.direction, rotation);
        ant.direction = Vector2Normalize(ant.direction);
    }

    if (ant.marker_cooldown <= 0)
    {
        cell_t &cell = field[row][col];
        
        ant.marker_cooldown = ANT_MARKER_BASE;

        switch (ant.state)
        {
            case GOING_HOME:
            {
                cell.food_marker = 255;

                break;
            }
            case SEARCHING_FOOD:
            {
                cell.home_marker = 255;
                
                break;
            }
        }
    }

    if (ant.attraction_cooldown <= 0)
    {
        cell_t *attractor = find_attractive_cell2(ant, field);

        if (attractor != nullptr)
        {
            Vector2 attractor_screen_pos;
           
            attractor_screen_pos.x = attractor->x * cell_radius + cell_radius / 2.0f;
            attractor_screen_pos.y = attractor->y * cell_radius + cell_radius / 2.0f;

            Vector2 to_attractor = attractor_screen_pos - ant.position;
            
            if (Vector2LengthSqr(to_attractor) > 0.00001) to_attractor = Vector2Normalize(to_attractor);
            
            ant.direction = to_attractor;
        }

        ant.attraction_cooldown = ANT_ATTRACTION_BASE;
    }

    ant.position += ant.direction * (ant.speed * dt);

    if (ant.position.x >= WindowsWidth || ant.position.x <= 0) ant.direction.x *= -1;
    if (ant.position.y >= WindowsHeight || ant.position.y <= 0) ant.direction.y *= -1;

    for (auto &source : sources)
    {
        if (CheckCollisionPointCircle(ant.position, source.position, source.radius))
        {
            ant.state = GOING_HOME;

            break;
        }
    }

    if (CheckCollisionPointCircle(ant.position, colony.position, colony.radius)) ant.state = SEARCHING_FOOD;
}

int main()
{
    InitWindow(WindowsWidth, WindowsHeight, "Ants Colony");
    SetTargetFPS(60);

    Vector2 cells;
    cells.x = WindowsWidth / cell_radius;
    cells.y = WindowsHeight / cell_radius;

    field_t field((int) cells.x, std::vector<cell_t>((int) cells.y));

    colony_t colony;
    colony.position = { 200, 200 };
    colony.radius = 20;
    colony.size = 1000;

    std::vector<ant_t> ants;

    for (int i = 0; i < colony.size; ++i)
    {
        ant_t ant;
        ant.position = colony.position;
        ant.state = SEARCHING_FOOD;
        ant.direction = {1, 0};

        float rotation = rand() % 360;

        ant.direction = Vector2Rotate(ant.direction, rotation * DEG2RAD);
        ant.speed = 60;
        ant.dir_cooldown = ANT_WANDER_BASE;
        ant.marker_cooldown = ANT_MARKER_BASE;
        ant.attraction_cooldown = ANT_ATTRACTION_BASE;

        ants.push_back(ant);
    }

    std::vector<food_source_t> sources;

    for (int i = 0; i < 1; ++i)
    {
        food_source_t source;
        source.charges = 100;
        source.radius = 40;
        source.position = {600, 400};
        sources.push_back(source);
    }

    for (int y = 0; y < (int) cells.y; ++y)
    {
        for (int x = 0; x < (int) cells.x; ++x)
        {
            cell_t &cell = field[x][y];
            cell.x = x;
            cell.y = y;
            cell.home_marker = 0;
            cell.food_marker = 0;
        }
    }

    float fadeout_timeout = MARKER_FADEOUT_BASE;

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
    
        BeginDrawing();
        {
            ClearBackground(BLACK);

            for (int x = 0; x < (int) cells.x; ++x)
            {
                for (int y = 0; y < (int) cells.y; ++y)
                {
                    cell_t cell = field[x][y];
                    
                    auto r = cell.home_marker;
                    auto g = cell.food_marker;
                    
                    Color color = { r, g, 0, 255 };

                    DrawRectangle(x * cell_radius, y * cell_radius, cell_radius, cell_radius, color);
                }
            }

            DrawCircleV(colony.position, colony.radius, RED);
            
            for (auto &source : sources)
            {
                DrawCircleV(source.position, source.radius, GREEN);
            }

            for (auto &ant : ants)
            {
                ant_update(ant, field, sources, colony, dt);
                DrawCircleV(ant.position, 2, WHITE);

                if (ant.state == GOING_HOME && false)
                {
                    float scale = cell_radius * 20;

                    Vector2 center = ant.position + ant.direction * scale;
                    Vector2 left = ant.position + Vector2Rotate(ant.direction,  45 * DEG2RAD) * scale;
                    Vector2 right = ant.position + Vector2Rotate(ant.direction, -45 * DEG2RAD) * scale;

                    attractive_points[0] = center;
                    attractive_points[1] = left;
                    attractive_points[2] = right;

                    DrawCircleV(attractive_points[0], 2, BLUE);
                    DrawCircleV(attractive_points[1], 2, BLUE);
                    DrawCircleV(attractive_points[2], 2, BLUE);
                }
            }
        }

        EndDrawing();

        fadeout_timeout -= dt;

        if (fadeout_timeout <= 0)
        {
            for (int y = 0; y < (int) cells.y; ++y)
            {
                for (int x = 0; x < (int) cells.x; ++x)
                {
                    cell_t &cell = field[x][y];

                    if (cell.home_marker > 0) cell.home_marker -= 1;
                    if (cell.food_marker > 0) cell.food_marker -= 1;
                }
            }

            fadeout_timeout = MARKER_FADEOUT_BASE;
        }
    }

    CloseWindow();
    
    return 0;
}
