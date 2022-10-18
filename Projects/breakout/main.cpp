#include <iostream>
#include <algorithm>
#include <vector>
#include <chrono>
#include <cstdint>
#include <thread>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <raylib-ext.hpp>

#define ENEMY_W 100
#define ENEMY_H 30
#define GAP 10
#define MIN_PADDING 30

#define BALL_W 60
#define BALL_H 60
#define BAR_W 200
#define BAR_H 30
#define DECORATION_HEIGHT 29

Vector2 ball_speed = {450, 450};

int monitor_w;
int monitor_h;

struct Window {
    GLFWwindow *handle;
    int width;
    int height;
    Vector2 pos;
    bool decorated;
    bool active;

    Color fill_color;

    Window() = default;

    Window(int window_w, int window_h, Vector2 pos, bool decorated = true,
        bool resizable = false) : width(window_w), height(window_h), pos(pos),
            decorated(decorated),
            active(true),
            fill_color({0, 0, 0, 255})
    {
        if (decorated) {
            this->pos.y -= DECORATION_HEIGHT;
            this->height += DECORATION_HEIGHT;
        }

        glfwWindowHint(GLFW_RESIZABLE, resizable);
        glfwWindowHint(GLFW_DECORATED, decorated);
        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GL_TRUE);
        glfwWindowHint(GLFW_FLOATING, GL_TRUE);

        this->handle = glfwCreateWindow(window_w, window_h, "", NULL, NULL);

        if (this->handle == NULL) {
            std::cout << "ERROR: can't create GLFW window\n" << std::endl;
            glfwTerminate();
        }

        glfwMakeContextCurrent(this->handle);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cout << "ERROR: can't start GLAD\n" << std::endl;
            glfwTerminate();
        }

        glViewport(0, 0, window_w, window_h);
        fill(this->fill_color);

        setPosition(pos);
    }

    void setPosition(Vector2 pos) {
        this->pos = pos;
        glfwMakeContextCurrent(this->handle);
        glfwSetWindowPos(this->handle, pos.x, pos.y + (decorated ? DECORATION_HEIGHT : 0));
    }

    void move(Vector2 move) {
        setPosition(this->pos + move);
    }

    void fill(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        this->fill_color = {r, g, b, a};
        glfwMakeContextCurrent(this->handle);
        glClearColor(
            r / 255.0f,
            g / 255.0f,
            b / 255.0f,
            a / 255.0f
        );
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glfwSwapBuffers(this->handle);
    }

    void fill(Color c) {
        fill(c.r, c.g, c.b, c.a);
    }

    void updateSize() {
        glfwGetWindowSize(this->handle, &this->width, &this->height);
        fill(this->fill_color);
    }

    void close() {
        glfwDestroyWindow(this->handle);
        this->active = false;
    }
};

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

struct Clock
{
    Clock(float target_fps) :
        dt(1.0f / 60.0f),
        ns_count(get_ns()),
        target_ns(1.0f / target_fps * float(1e9)) {}

    float dt;
    void start()
    {
        ns_count = this->get_ns();
    }

    void tick()
    {
        uint64_t dt_end = get_ns();
        uint64_t ns_delta = dt_end - ns_count;
        std::this_thread::sleep_for(
            std::chrono::nanoseconds(target_ns - ns_delta)
        );

        dt_end = get_ns();
        dt = (dt_end - ns_count) / float(1e9);
        ns_count = dt_end;
    }

private:
    uint64_t ns_count;
    uint64_t target_ns;
    uint64_t get_ns()
    {
        using namespace std::chrono;
        return duration_cast<nanoseconds>(
            system_clock::now().time_since_epoch()
        ).count();
    }
};

int
main ()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    int count;
    GLFWmonitor **monitor = glfwGetMonitors(&count);
    const GLFWvidmode *video_mode = glfwGetVideoMode(monitor[0]);
    monitor_w = video_mode->width;
    monitor_h = video_mode->height;

    int enemies_width_count = (monitor_w + GAP - MIN_PADDING * 2) / (ENEMY_W + GAP);
    int padding = (monitor_w - (ENEMY_W + GAP) * enemies_width_count - GAP) / 2;
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
        float((monitor_w - BAR_W) / 2),
        float(monitor_h - BAR_H - 100)
    }, true, true);
    Window ball(BALL_W, BALL_H, {
        float((monitor_w - BALL_W) / 2),
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

    float dt = 0.016;
    auto clock = Clock(60);
    clock.start();
    while (!glfwWindowShouldClose(bar.handle)) {
        // TODO(aguschin): Поместить код обновления позиции в функцию и
        // вызывать её для всех окон (можно будет передвигать врагов)
        int bar_x, bar_y;
        glfwGetWindowPos(bar.handle, &bar_x, &bar_y);
        bar.setPosition(Vector2 {
            float(bar_x),
            float(bar_y - DECORATION_HEIGHT)
        });
        bar.updateSize();

        ball.move(ball_speed * dt);
        if (ball.pos.x < 0 || ball.pos.x + ball.width >= monitor_w)
            ball_speed.x *= -1;
        if (ball.pos.y < 0 || ball.pos.y + ball.height >= monitor_h)
            ball_speed.y *= -1;
        ball.pos.x = Clamp(ball.pos.x, 0, monitor_w - ball.width);
        ball.pos.y = Clamp(ball.pos.y, 0, monitor_h - ball.height);

        for (int i = 0; i < enemies_rows; ++i) {
            for (int j = 0; j < enemies_width_count; ++j) {
                Window *enemy = &enemies[i][j];
                if (enemy->active && reflect(ball, *enemy, ball_speed)) {
                    enemy->close();
                }
            }
        }
        reflect(ball, bar, ball_speed);

        glfwPollEvents();

        clock.tick();
        dt = clock.dt;
    }

    glfwTerminate();
    return (EXIT_SUCCESS);
}
