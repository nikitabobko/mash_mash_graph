#version 330

#define float2 vec2
#define float3 vec3
#define float4 vec4
#define float4x4 mat4
#define float3x3 mat3

in float2 fragmentTexCoord;

layout(location = 0) out vec4 fragColor;

uniform int g_screenWidth;
uniform int g_screenHeight;

int SCENE_MAX = 10000;

uniform float3 g_bBoxMin   = float3(-1,-1,-1);
uniform float3 g_bBoxMax   = float3(+1,+1,+1);

uniform float4 backgroundColor = float4(0,0,0,1);

uniform vec3 cam_pos;

uniform vec3 cam_dir;

uniform vec3 cam_y;

uniform vec3 cam_x;

vec4 WHITE = vec4(1, 1, 1, 0);
vec4 HALF_WHITE = vec4(0.5, 0.5, 0.5, 0);
vec4 QUAD_WHITE = vec4(0.25, 0.25, 0.25, 0);
vec4 BLACK = vec4(0, 0, 0, 0);
vec4 INDIGO = vec4(75.f/255,0,130.f/255,0);

float MIN_DIST = 0.01;

struct Scene {
    float closest_dist;
    vec4 color;
};

Scene sphere_scene(vec3 p, vec3 center, float radius, vec4 color) {
    Scene ret;
    ret.closest_dist = length(p - center) - radius;
    ret.color = color;
    return ret;
}

Scene box_scene(vec3 p, vec3 center, vec3 lengths, vec4 color) {
    Scene ret;
    ret.closest_dist = length(max(abs(p-center)-lengths,0.0));
    ret.color = color;
    return ret;
}

Scene torus_scene(vec3 p, vec3 center, vec2 t, vec4 color) {
    p = p - center;
    Scene ret;
    ret.closest_dist = length(vec2(length(p.xz)-t.x,p.y))-t.y;
    ret.color = color;
    return ret;
}

Scene y_chess_plane_scene(vec3 p, float y, vec2 zhopa, vec4 color1, vec4 color2) {
    Scene ret = box_scene(p, vec3(0, y, 0), vec3(zhopa.x, 0, zhopa.y), BLACK);
    if ((int((p.x + SCENE_MAX) / 100) + int((p.z + SCENE_MAX) / 100)) % 2 == 0) {
        ret.color = color1;
    } else {
        ret.color = color2;
    }
    return ret;
}

Scene scene0(vec3 point) {
    Scene[] scenes = Scene[](
        sphere_scene(point, vec3(0, 200, 100), 100, vec4(0.2, 0.2, 0.2, 0)),
        sphere_scene(point, vec3(-500, -50, 0), 300, vec4(0.4, 0.2, 0.2, 0)),
        box_scene(point, vec3(400, -400, -500), vec3(100, 200, 200), vec4(0.2, 0.4, 0.2, 0)),
        torus_scene(point, vec3(100, -400, 300), vec2(1000, 50), vec4(0.2, 0.2, 0.2, 0)),
        y_chess_plane_scene(point, -700, vec2(2000, 2000), vec4(0.2, 0.2, 0.1, 10), BLACK)
    );
    Scene cur = scenes[0];
    for (int i = 0; i < scenes.length(); ++i) {
        if (scenes[i].closest_dist < cur.closest_dist) {
            cur = scenes[i];
        }
    }
    return cur;
}

Scene cur_scene(vec3 point) {
    return scene0(point);
}

vec3 estimateNormal(float3 z) {
    float eps = 0.001;
    float3 z1 = z + float3(eps, 0, 0);
    float3 z2 = z - float3(eps, 0, 0);
    float3 z3 = z + float3(0, eps, 0);
    float3 z4 = z - float3(0, eps, 0);
    float3 z5 = z + float3(0, 0, eps);
    float3 z6 = z - float3(0, 0, eps);
    float dx = cur_scene(z1).closest_dist - cur_scene(z2).closest_dist;
    float dy = cur_scene(z3).closest_dist - cur_scene(z4).closest_dist;
    float dz = cur_scene(z5).closest_dist - cur_scene(z6).closest_dist;
    return normalize(float3(dx, dy, dz) / (2.0*eps));
}

bool isVisible(vec3 from, vec3 to) {
    vec3 direction = normalize(to - from);
    float step = min(cur_scene(from).closest_dist, length(to - from));
    vec3 cur = from + direction*step;

    step = min(cur_scene(cur).closest_dist, length(to - cur));
    while(step > MIN_DIST) {
        cur += direction*step;
        step = min(cur_scene(cur).closest_dist, length(to - cur));
    }

    return length(to - cur) <= 100*MIN_DIST;
}

struct Light {
    vec4 intensity;
    vec3 point;
};

vec4 ligth_point_scene0(vec3 point) {
    Light[] ligths = Light[](
        Light(QUAD_WHITE, vec3(900, 600, -200)),
        Light(QUAD_WHITE, vec3(-500, 300, -800))
    );
    vec4 color = BLACK;
    for (int i = 0; i < ligths.length(); ++i) {
        Light light = ligths[i];
        if (isVisible(light.point, point)) {
            vec3 normal = estimateNormal(point);
            color += light.intensity*max(dot(normalize(light.point - point), normal), 0.f);
//              float dist = length(light.point - point);
//              color += 100000/(dist*dist);
        }
    }
    return color;
}

vec4 light_point(vec3 point) {
    return ligth_point_scene0(point);
}

vec3 EyeRayDir(float x, float y, float w) {
    float fov = 3.141592654f/(2.0f);
    float3 ray_dir;

    return normalize(cam_x*x + cam_y*y + cam_dir*(w)/tan(fov/2.0f));
}

bool isOutOfScene(vec3 point) {
    return abs(point.x) > SCENE_MAX || abs(point.y) > SCENE_MAX || abs(point.z) > SCENE_MAX;
}

vec4 RayTrace(float x, float y, vec3 ray_dir, float w, float h) {
    vec3 cur = cam_pos;
    vec4 color = backgroundColor;
    while (!isOutOfScene(cur)) {
        Scene scene = cur_scene(cur);
        if (scene.closest_dist <= MIN_DIST) {
            color = scene.color + light_point(cur);
            break;
        }
        cur += scene.closest_dist*ray_dir;
    }
    return color;
}

void main(void) {
    float w = float(g_screenWidth);
    float h = float(g_screenHeight);

    float x = fragmentTexCoord.x*w - w/2;
    float y = fragmentTexCoord.y*h - h/2;

    float3 ray_dir = EyeRayDir(x,y, w);

    fragColor = RayTrace(x, y, ray_dir, w, h);
}
