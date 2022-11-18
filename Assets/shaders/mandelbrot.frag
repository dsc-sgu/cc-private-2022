#version 330
#define M_PI 3.1415926535897932384626433832795

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

uniform vec2 resolution;
uniform vec4 viewport;
uniform float time;

vec2 product(vec2 a, vec2 b)
{
    return vec2(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
}

vec2 powc(vec2 c, int p)
{
    vec2 res = c;
    for (int i = 1; i < p; ++i)
        res = product(res, c);
    return res;
}

float f(float x, float q, float p)
{
    float a = cos(sqrt(x) * q + p);
    return a * a;
}

// All components are in the range [0…1], including hue.
vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

// All components are in the range [0…1], including hue.
vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec3 f2(int iterations)
{
    return hsv2rgb(vec3(float(int(iterations * 10) % 256) / 256.0, 1.0, 1.0));
}

vec3 f3(int iterations)
{
    return hsv2rgb(vec3(
        ( sin(float(iterations))
        + sin(float(iterations) / 3.0)
        + sin(float(iterations) / 5.0))
        * time * 0.5,
        1.0,
        1.0
    ));
}

void main()
{
    vec2 uv = fragTexCoord * (viewport.yw - viewport.xz) + viewport.xz;

    vec2 z = vec2(0.0, 0.0);
    vec2 c = uv.xy;

    int iterations = 0;
    bool inside = true;
    for (int i = 0; i < 200; i++)
    {
        z = powc(z, 2) + c;

        iterations += 1;
        if (dot(z, z) > 4.0)
        {
            inside = false;
            break;
        }
    }

    if (inside)
    {
        float k = mod(time * 0.8, M_PI);
        finalColor = vec4(
            f(float(iterations), 1.0, k + 0.0),
            f(float(iterations), 1.0, k + 120.0),
            f(float(iterations), 1.0, k + 240.0),
            1.0
        );
    }
    else if (true)
    {
        float k = mod(time * 0.8, M_PI);
        finalColor = vec4(
            f(float(iterations), 1.0, k + 0.0),
            f(float(iterations), 1.0, k + 120.0),
            f(float(iterations), 1.0, k + 240.0),
            1.0
        );
    }
    else if (false)
    {
        float k = pow(time, 2) * 0.003;
        finalColor = vec4(
            f(float(iterations), 1.0, k * 0.0),
            f(float(iterations), 1.0, k * 120.0),
            f(float(iterations), 1.0, k * 240.0),
            1.0
        );
    }
    else if (true)
    {
        finalColor = vec4(f3(iterations), 1.0);
    }
}
