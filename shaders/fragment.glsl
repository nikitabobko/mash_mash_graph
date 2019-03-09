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

// Colors
vec4 WHITE = vec4(1, 1, 1, 0);
vec4 HALF_WHITE = vec4(0.5, 0.5, 0.5, 0);
vec4 QUAD_WHITE = vec4(0.25, 0.25, 0.25, 0);
vec4 BLACK = vec4(0, 0, 0, 0);
vec4 INDIGO = vec4(75.f/255,0,130.f/255,0);
vec4 GREY = vec4(0.2, 0.2, 0.2, 0);

// Ambient
vec4 NO_AMBIENT = BLACK;
vec4 GREEEN_RUBBER_AMBIENT = vec4(0.0, 0.05, 0.0, 0);

// Diffuse
vec4 GREEEN_RUBBER_DIFFUSE = vec4(0.4, 0.5, 0.4, 0.f);

// Specular
vec4 NO_SPECULAR = BLACK;
vec4 GREEEN_RUBBER_SPECULAR = vec4(0.04, 0.7, 0.04, 0.f);
vec4 EMERALD_SPECULAR = vec4(0.633, 0.727811, 0.633, 0);
float EMERALD_SPECULAR_SHINESS = 0.6;

// Specualr shiness
float NO_SPECULAR_SHINESS = 0.f;
float GREEEN_RUBBER_SPECULAR_SHINESS = 0.078125;

float MIN_DIST = 0.01;

struct Object {
    float dist;
    vec4 ambient;
    vec4 diffuse;
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

float triangular_prism_dist(vec3 p, vec3 center, vec2 h) {
    p -= center;
    vec3 q = abs(p);
    return max(q.z-h.y,max(q.x*0.866025+p.y*0.5,-p.y)-h.x*0.5);
}

float ellipsoid_dist(vec3 p, vec3 center, vec3 r) {
    p -= center;
    return (length( p/r ) - 1.0) * min(min(r.x,r.y),r.z);
}

//float dist_displace(float dist, vec3 p) {
//    return dist + displacement(p);
//}


Object y_chess_plane_object(vec3 p, float y, vec2 lengths, Object obj1, Object obj2) {
    float dist = box_dist(p, vec3(0, y, 0), vec3(lengths.x, 0, lengths.y));
    if ((int((p.x + SCENE_MAX) / 100) + int((p.z + SCENE_MAX) / 100)) % 2 == 0) {
        obj1.dist = dist;
        return obj1;
    } else {
        obj2.dist = dist;
        return obj2;
    }
}

Object emerald_object(float dist) {
    return Object(dist, vec4(0.0215, 0.1745, 0.0215, 0), vec4(0.07568, 0.61424, 0.07568, 0), vec4(0.633, 0.727811, 0.633, 0), 0.6);
}

Object green_ruber_object(float dist) {
    return Object(dist, GREEEN_RUBBER_AMBIENT, GREEEN_RUBBER_DIFFUSE, GREEEN_RUBBER_SPECULAR, GREEEN_RUBBER_SPECULAR_SHINESS);
}

Object ruby_object(float dist) {
    return Object(dist, vec4(0.1745, 0.01175, 0.01175, 0), vec4(0.61424, 0.04136, 0.04136, 0), vec4(0.727811, 0.626959, 0.626959, 0), 0.6);
}

Object gold_object(float dist) {
    return Object(dist, vec4(0.24725, 0.1995, 0.0745, 0.f), vec4(0.75164, 0.60648, 0.22648, 0.f), vec4(0.628281, 0.555802, 0.366065, 0.f), 0.4);
}

Object red_plastic(float dist) {
    return Object(dist, vec4(0.0, 0.0, 0.0, 0), vec4(0.5, 0.0, 0.0, 0), vec4(0.7, 0.6, 0.6, 0), 0.25);
}

Object white_plastic(float dist) {
    return Object(dist, vec4(0.0, 0.0, 0.0, 0), vec4(0.55, 0.55, 0.55, 0), vec4(0.70, 0.70, 0.70, 0), .25);
}

Object black_plastic(float dist) {
    return Object(dist, vec4(0.0, 0.0, 0.0, 0), vec4(0.01, 0.01, 0.01, 0), vec4(0.50, 0.50, 0.50, 0), .25);
}

Object matt_green_object(float dist) {
    return Object(dist, GREEEN_RUBBER_AMBIENT, GREEEN_RUBBER_DIFFUSE, NO_SPECULAR, NO_SPECULAR_SHINESS);
}

Object chrome_object(float dist) {
    return Object(dist, vec4(0.25, 0.25, 0.25, 0), vec4(0.4, 0.4, 0.4, 0), vec4(0.774597, 0.774597, 0.774597, 0), 0.6);
}

//float displacement(vec3 p) {
//    return sin(20*p.x)*sin(20*p.y)*sin(20*p.z);
//}

Object scene0(vec3 p) {
    Object[] scenes = Object[](
        ruby_object(sphere_dist(p, vec3(0, 200, 100), 100)),
        matt_green_object(sphere_dist(p, vec3(-500, -50, 0), 300)),
        red_plastic(box_dist(p, vec3(400, -400, -500), vec3(100, 200, 200))),
        gold_object(torus_dist(p, vec3(100, -400, 300), vec2(1000, 50))),
        y_chess_plane_object(p, -700, vec2(2000, 2000), white_plastic(0), red_plastic(0)),
        emerald_object(triangular_prism_dist(p, vec3(0, -500, 200), vec2(300, 100))),
        chrome_object(ellipsoid_dist(p, vec3(-500, 200, -600), vec3(100, 200, 100)))
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
    vec4 ambinet_color = WHITE;
    Light[] ligths = Light[](
        Light(WHITE, vec3(900, 600, -200)),
        Light(WHITE, vec3(-500, 300, -800)),
        Light(WHITE, vec3(0, -2000, 0))
    );
    vec4 color = BLACK;
    for (int i = 0; i < ligths.length(); ++i) {
        Light light = ligths[i];
        vec3 normal = estimateNormal(point);
        vec3 to_light = normalize(light.point - point);
        float scalar = dot(to_light, normal);
        color += ambinet_color*obj.ambient;
        if (isVisible(light.point, point)) {
            color += light.color*obj.diffuse*max(scalar, 0.f);
            vec3 reflected_light = 2*scalar*length(to_light)*normal - to_light;
            if (obj.specular != NO_SPECULAR) {
                color += light.color*obj.specular*pow(max(dot(-ray_dir, reflected_light), 0.f), 128*obj.specular_shiness);
            }
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
            color = light_point(cur, obj, ray_dir);
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
