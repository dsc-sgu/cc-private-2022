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

void basic(vec2 z, vec2 c)
{
    int iterations = 0;
    bool inside = true;
    for (int i = 0; i < 1000; i++)
    {
        z = powc(z, 2) + c;

        iterations += 1;
        if (dot(z, z) > 4.0)
        {
            inside = false;
            break;
        }
    }
    if (inside) finalColor = vec4(vec3(0.0), 1.0);
    else
    {
        float k = mod(time * 0.8, M_PI);
        finalColor = vec4(
            f(float(iterations), 1.0, k + 0.0),
            f(float(iterations), 1.0, k + 120.0),
            f(float(iterations), 1.0, k + 240.0),
            1.0
        );
    }
}

void smooth_color(vec2 z, vec2 c)
{
    float val = 0.0;
    for(int n = 0; n < 1000; n++)
    {
        float tmp = z.x;
        z.x = (z.x * z.x - z.y * z.y) + c.x;
        z.y = (z.y * tmp) * 2 + c.y;
        if (dot(z, z) > 4.0)
        {
            val = float(n) + 1.0 - log(log(length(z))) / log(2.0);
            break;
        }
    }
    if (val == 0.0) finalColor = vec4(vec3(0.0), 1.0);
    else
    {
        float k = mod(time * 0.8, M_PI);
        finalColor = vec4(
            f(val, 1.0, k + 0.0),
            f(val, 1.0, k + 120.0),
            f(val, 1.0, k + 240.0),
            1.0
        );
    }
}

void main()
{
    vec2 uv = fragTexCoord * (viewport.yw - viewport.xz) + viewport.xz;

    vec2 z = vec2(0.0, 0.0);
    vec2 c = uv.xy;

    smooth_color(z, c);
    // basic(z, c);
}
