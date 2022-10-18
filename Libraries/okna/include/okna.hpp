#ifndef OKNA_HPP
#define OKNA_HPP

#include <cstdint>
#include <raylib-ext.hpp>

struct GLFWwindow;

struct Window {
    GLFWwindow *handle;
    int width;
    int height;
    Vector2 pos;
    bool decorated;
    bool resizable;
    bool active;
    Color fill_color;

    Window();
    Window(int window_w, int window_h, Vector2 pos, bool decorated = true,
        bool resizable = false);
    void setPosition(Vector2 pos);
    void move(Vector2 move);
    void fill(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
    void fill(Color c);
    void close();
    void syncSize();
    void syncPosition();
    void sync();
};

struct Clock
{
    float dt;

    Clock(float target_fps);
    void start();
    void tick();

private:
    uint64_t ns_count;
    uint64_t target_ns;

    uint64_t get_ns();
};

void oknaInit();
void oknaTerminate();
Vector2 oknaGetMonitorSize();

#endif // OKNA_HPP
