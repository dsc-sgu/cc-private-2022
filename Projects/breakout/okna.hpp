#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <raylib-ext.hpp>

#define DECORATION_HEIGHT 29

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

void
init_graphics()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
}

void
terminate_graphics()
{
    glfwTerminate();
}

void
update_graphics()
{
    glfwPollEvents();
}

Vector2
get_monitor_size()
{
    int count;
    GLFWmonitor **monitor = glfwGetMonitors(&count);
    const GLFWvidmode *video_mode = glfwGetVideoMode(monitor[0]);
    return {
        float(video_mode->width),
        float(video_mode->height),
    };
}
