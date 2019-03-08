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

// Specular
vec4 NO_SPECULAR = BLACK;
float NO_SPECULAR_SHINESS = 0.f;
vec4 SPECULAR_EMERALD = vec4(0.633, 0.727811, 0.633, 0);

float MIN_DIST = 0.01;

struct Object {
    float dist;
    vec4 color;
    vec4 specular;
    float specular_shiness;
};

float sphere_dist(vec3 p, vec3 center, float radius) {
    return length(p - center) - radius;
}

float box_dist(vec3 p, vec3 center, vec3 lengths) {
    return length(max(abs(p-center)-lengths,0.0));
}

float torus_dist(vec3 p, vec3 center, vec2 t) {
    p -= center;
    return length(vec2(length(p.xz)-t.x,p.y))-t.y;
}

Object y_chess_plane_object(vec3 p, float y, vec2 zhopa, vec4 color1, vec4 color2, vec4 specular, float specular_shiness) {
    Object ret = Object(box_dist(p, vec3(0, y, 0), vec3(zhopa.x, 0, zhopa.y)), BLACK, specular, specular_shiness);
    if ((int((p.x + SCENE_MAX) / 100) + int((p.z + SCENE_MAX) / 100)) % 2 == 0) {
        ret.color = color1;
    } else {
        ret.color = color2;
    }
    return ret;
}

Object scene0(vec3 point) {
    Object[] scenes = Object[](
        Object(sphere_dist(point, vec3(0, 200, 100), 100), vec4(0.2, 0.2, 0.2, 0), NO_SPECULAR, NO_SPECULAR_SHINESS),
        Object(sphere_dist(point, vec3(-500, -50, 0), 300), vec4(0.4, 0.2, 0.2, 0), NO_SPECULAR, NO_SPECULAR_SHINESS),
        Object(box_dist(point, vec3(400, -400, -500), vec3(100, 200, 200)), vec4(0.2, 0.4, 0.2, 0), NO_SPECULAR, NO_SPECULAR_SHINESS),
        Object(torus_dist(point, vec3(100, -400, 300), vec2(1000, 50)), vec4(0.2, 0.2, 0.2, 0), NO_SPECULAR, NO_SPECULAR_SHINESS),
        y_chess_plane_object(point, -700, vec2(2000, 2000), vec4(0.2, 0.2, 0.1, 10), BLACK, NO_SPECULAR, NO_SPECULAR_SHINESS)
    );
    Object cur = scenes[0];
    for (int i = 1; i < scenes.length(); ++i) {
        if (scenes[i].dist < cur.dist) {
            cur = scenes[i];
        }
    }
    return cur;
}

Object cur_scene(vec3 point) {
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
    float dx = cur_scene(z1).dist - cur_scene(z2).dist;
    float dy = cur_scene(z3).dist - cur_scene(z4).dist;
    float dz = cur_scene(z5).dist - cur_scene(z6).dist;
    return normalize(float3(dx, dy, dz) / (2.0*eps));
}

bool isVisible(vec3 from, vec3 to) {
    vec3 direction = normalize(to - from);
    float step = min(cur_scene(from).dist, length(to - from));
    vec3 cur = from + direction*step;

    step = min(cur_scene(cur).dist, length(to - cur));
    while(step > MIN_DIST) {
        cur += direction*step;
        step = min(cur_scene(cur).dist, length(to - cur));
    }

    return length(to - cur) <= 100*MIN_DIST;
}

struct Light {
    vec4 color;
    vec3 point;
};

vec4 ligth_point_scene0(vec3 point, Object obj, vec3 ray_dir) {
    Light[] ligths = Light[](
        Light(QUAD_WHITE, vec3(900, 600, -200)),
        Light(QUAD_WHITE, vec3(-500, 300, -800))
    );
    vec4 color = BLACK;
    for (int i = 0; i < ligths.length(); ++i) {
        Light light = ligths[i];
        vec3 normal = estimateNormal(point);
        vec3 to_light = normalize(light.point - point);
        float scalar = dot(to_light, normal);
        if (isVisible(light.point, point)) {
            color += light.color*max(scalar, 0.f);
            vec3 reflected_light = 2*scalar*length(to_light)*normal - to_light;
            color += obj.specular*pow(max(dot(-ray_dir, reflected_light), 0.f), obj.specular_shiness);
//              float dist = length(light.point - point);
//              color += 100000/(dist*dist);
        }

    }
    return color;
}

vec4 light_point(vec3 point, Object obj, vec3 ray_dir) {
    return ligth_point_scene0(point, obj, ray_dir);
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
        Object obj = cur_scene(cur);
        if (obj.dist <= MIN_DIST) {
            color = obj.color + light_point(cur, obj, ray_dir);
            break;
        }
        cur += obj.dist*ray_dir;
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
