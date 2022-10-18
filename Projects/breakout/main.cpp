#include <iostream>
#include <algorithm>
#include <vector>

#include <raylib-ext.hpp>
#include "okna.hpp"

#define ENEMY_W 100
#define ENEMY_H 30
#define GAP 10
#define MIN_PADDING 30

#define BALL_W 60
#define BALL_H 60
#define BAR_W 200
#define BAR_H 30

Vector2 ball_speed = {450, 450};

enum Collision {
    COLLISION_LEFT,
    COLLISION_BOTTOM,
    COLLISION_TOP,
    COLLISION_RIGHT,
    COLLISION_NONE
};

Collision collide(const Window& a, const Window& b) {
    // NOTE (mchernigin): Collides a WITH b (not symmetric)!
    if (!(a.pos.x + a.width < b.pos.x || a.pos.x > b.pos.x + b.width ||
          a.pos.y + a.height < b.pos.y || a.pos.y > b.pos.y + b.height)) {

        float dx1 = abs(a.pos.x + a.width - b.pos.x);
        float dx2 = abs(b.pos.x + b.width - a.pos.x);
        float dx = std::min({dx1, dx2, dx1 + dx2 - std::max(a.width, b.width)});

        float dy1 = abs(a.pos.y + a.height - b.pos.y);
        float dy2 = abs(b.pos.y + b.height - a.pos.y);
        float dy = std::min({dy1, dy2, dy1 + dy2 - std::max(a.height, b.height)});

        if (dx > dy) {
            float dist_t = a.pos.y + a.height - b.pos.y;
            float dist_b = b.pos.y + b.height - a.pos.y;
            return (dist_t <= dist_b ? COLLISION_TOP : COLLISION_BOTTOM);
        } else {
            float dist_l = a.pos.x + a.width - b.pos.x;
            float dist_r = b.pos.x + b.width - a.pos.x;
            return (dist_l <= dist_r ? COLLISION_LEFT : COLLISION_RIGHT);
        }
    }

    return (COLLISION_NONE);
}

bool
reflect(Window &ball, Window &obj, Vector2 &ball_speed)
{
    switch (collide(ball, obj)) {
    case COLLISION_LEFT:
        ball_speed.x = -abs(ball_speed.x);
        ball.setPosition(Vector2 { obj.pos.x - ball.width, ball.pos.y });
        break;
    case COLLISION_RIGHT:
        ball_speed.x = abs(ball_speed.x);
        ball.setPosition(Vector2 { obj.pos.x + obj.width, ball.pos.y });
        break;
    case COLLISION_TOP:
        ball_speed.y = -abs(ball_speed.y);
        ball.setPosition(Vector2 { ball.pos.x, obj.pos.y - ball.height });
        break;
    case COLLISION_BOTTOM:
        ball_speed.y = abs(ball_speed.y);
        ball.setPosition(Vector2 { ball.pos.x, obj.pos.y + obj.height });
        break;
    case COLLISION_NONE:
        return false;
        break;
    }
    return true;
}

int
main()
{
    oknaInit();

    Vector2 monitor_dim = get_monitor_size();

    int enemies_width_count =
        (monitor_dim.x + GAP - MIN_PADDING * 2)
        / (ENEMY_W + GAP);
    int padding =
        (monitor_dim.x - (ENEMY_W + GAP) * enemies_width_count - GAP) / 2;
    int enemies_rows = 5;
    std::vector<std::vector<Window>> enemies(
        enemies_rows, std::vector<Window>(enemies_width_count)
    );

    Color colors[6] = {
        { 255,   0,   0, 255 },
        { 255, 128,   0, 255 },
        { 255, 255,   0, 255 },
        {   0, 255,   0, 255 },
        {   0,   0, 255, 255 },
        {   0, 128, 255, 255 },
    };
    for (int row = 0; row < enemies_rows; ++row)
    {
        for (int col = 0; col < enemies_width_count; ++col)
        {
            enemies[row][col] = Window(
                ENEMY_W, ENEMY_H, {
                    float(padding + (ENEMY_W + GAP) * col),
                    float(40 + (ENEMY_H + DECORATION_HEIGHT + GAP) * row)
                }
            );
        }
    }

    Window bar(BAR_W, BAR_H, Vector2 {
        float((monitor_dim.x - BAR_W) / 2),
        float(monitor_dim.y - BAR_H - 100)
    }, true, true);
    Window ball(BALL_W, BALL_H, {
        float((monitor_dim.x - BALL_W) / 2),
        float(bar.pos.y - BALL_H)
    }, false);

    for (int row = 0; row < enemies_rows; ++row)
    {
        for (int col = 0; col < enemies_width_count; ++col)
        {
            enemies[row][col].fill(colors[row]);
        }
    }

    ball.fill(255, 0, 0, 255);
    bar.fill(0, 128, 128, 255);

    auto clock = Clock(60);
    clock.start();
    while (bar.active) {
        ball.move(ball_speed * clock.dt);
        if (ball.pos.x < 0 || ball.pos.x + ball.width >= monitor_dim.x)
            ball_speed.x *= -1;
        if (ball.pos.y < 0 || ball.pos.y + ball.height >= monitor_dim.y)
            ball_speed.y *= -1;
        ball.pos.x = Clamp(ball.pos.x, 0, monitor_dim.x - ball.width);
        ball.pos.y = Clamp(ball.pos.y, 0, monitor_dim.y - ball.height);

        for (int i = 0; i < enemies_rows; ++i) {
            for (int j = 0; j < enemies_width_count; ++j) {
                Window *enemy = &enemies[i][j];
                if (enemy->active && reflect(ball, *enemy, ball_speed)) {
                    enemy->close();
                }
            }
        }
        reflect(ball, bar, ball_speed);

        for (int i = 0; i < enemies_rows; ++i) {
            for (int j = 0; j < enemies_width_count; ++j) {
                Window *enemy = &enemies[i][j];
                if (enemy->active) {
                    enemy->sync();
                }
            }
        }
        bar.sync();
        // TODO(aguschin): Починить syncPosition для случая, когда
        // обновляется двигающееся само по себе окно.
        // ball.sync();

        clock.tick();
    }

    oknaTerminate();

    return (EXIT_SUCCESS);
}
