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

uniform float3 g_bBoxMin   = float3(-1,-1,-1);
uniform float3 g_bBoxMax   = float3(+1,+1,+1);

//uniform float4x4 g_rayMatrix;

uniform float4   backgroundColor = float4(0,0,0,1);

uniform float3 cam_pos;// = vec3(0, 0, 1500);

vec3 cam_dir = vec3(0, 0, -1000);

vec3 cam_y = vec3(0, 1, 0);

vec3 cam_x = vec3(1, 0, 0);

float4 WHITE = vec4(1, 1, 1, 0);

float MIN_DIST = 0.01;

struct Scene {
    float closest;
    vec4 color;
};

Scene sphere_scene(vec3 p, vec3 center, float radius, vec4 color) {
    Scene ret;
    ret.closest = length(p - center) - radius;
    ret.color = color;
    return ret;
}

Scene box_scene(vec3 p, vec3 center, vec3 lengths, vec4 color) {
    Scene ret;
    ret.closest = length(max(abs(p-center)-lengths,0.0));
    ret.color = color;
    return ret;
}

Scene torus_scene(vec3 p, vec3 center, vec2 t, vec4 color) {
    p = p - center;
    Scene ret;
    ret.closest = length(vec2(length(p.xz)-t.x,p.y))-t.y;
    ret.color = color;
    return ret;
}

Scene scene0(vec3 point) {
    Scene[] scenes = Scene[](
        sphere_scene(point, vec3(0, 200, 100), 100, vec4(0.2, 0.2, 0.2, 0)),
        sphere_scene(point, vec3(-500, -50, 0), 300, vec4(0.4, 0.2, 0.2, 0)),
        box_scene(point, vec3(400, -400, -500), vec3(100, 200, 200), vec4(0.2, 0.4, 0.2, 0)),
        torus_scene(point, vec3(100, -400, 300), vec2(100, 50), vec4(0.2, 0.2, 0.2, 0))
    );
    Scene cur = scenes[0];
    for (int i = 0; i < scenes.length(); ++i) {
        if (scenes[i].closest < cur.closest) {
            cur = scenes[i];
        }
    }
    return cur;
}

Scene cur_scene(vec3 point) {
    return scene0(point);
}

vec3[1] ligths0 = vec3[](vec3(900, 600, -200));

vec3[1] cur_lights() {
    return ligths0;
}

vec3 estimateNormal(float3 z) {
    float eps = 0.001;
    float3 z1 = z + float3(eps, 0, 0);
    float3 z2 = z - float3(eps, 0, 0);
    float3 z3 = z + float3(0, eps, 0);
    float3 z4 = z - float3(0, eps, 0);
    float3 z5 = z + float3(0, 0, eps);
    float3 z6 = z - float3(0, 0, eps);
    float dx = cur_scene(z1).closest - cur_scene(z2).closest;
    float dy = cur_scene(z3).closest - cur_scene(z4).closest;
    float dz = cur_scene(z5).closest - cur_scene(z6).closest;
    return normalize(float3(dx, dy, dz) / (2.0*eps));
}

float3 EyeRayDir(float x, float y, float w, float h) {
    return normalize(cam_dir + x*cam_x + y*cam_y);
//	float fov = 3.141592654f/(2.0f);
//    float3 ray_dir;
//
//	ray_dir.x = x+0.5f - (w/2.0f);
//	ray_dir.y = y+0.5f - (h/2.0f);
//	ray_dir.z = -(w)/tan(fov/2.0f);
//
//    return normalize(ray_dir);

}

bool isOutOfScene(vec3 point) {
    int max = 5000;
    return abs(point.x) > max || abs(point.y) > max || abs(point.z) > max;
}

//bool RayBoxIntersection(float3 ray_pos, float3 ray_dir, float3 boxMin, float3 boxMax, inout float tmin, inout float tmax)
//{
//  ray_dir.x = 1.0f/ray_dir.x;
//  ray_dir.y = 1.0f/ray_dir.y;
//  ray_dir.z = 1.0f/ray_dir.z;
//
//  float lo = ray_dir.x*(boxMin.x - ray_pos.x);
//  float hi = ray_dir.x*(boxMax.x - ray_pos.x);
//
//  tmin = min(lo, hi);
//  tmax = max(lo, hi);
//
//  float lo1 = ray_dir.y*(boxMin.y - ray_pos.y);
//  float hi1 = ray_dir.y*(boxMax.y - ray_pos.y);
//
//  tmin = max(tmin, min(lo1, hi1));
//  tmax = min(tmax, max(lo1, hi1));
//
//  float lo2 = ray_dir.z*(boxMin.z - ray_pos.z);
//  float hi2 = ray_dir.z*(boxMax.z - ray_pos.z);
//
//  tmin = max(tmin, min(lo2, hi2));
//  tmax = min(tmax, max(lo2, hi2));
//
//  return (tmin <= tmax) && (tmax > 0.f);
//}


//float3 RayMarchConstantFog(float tmin, float tmax, inout float alpha) {
//    float dt = 0.05f;
//	float t  = tmin;
//
//	alpha  = 1.0f;
//	float3 color = float3(0,0,0);
//
//	while(t < tmax && alpha > 0.01f)
//	{
//	  float a = 0.05f;
//	  color += a*alpha*float3(1.0f,1.0f,0.0f);
//	  alpha *= (1.0f-a);
//	  t += dt;
//	}
//
//	return color;
//}

bool isVisible(vec3 from, vec3 to) {
    vec3 direction = normalize(to - from);
    float step = min(cur_scene(from).closest, length(to - from));
    vec3 cur = from + direction*step;

    step = min(cur_scene(cur).closest, length(to - cur));
    while(step > MIN_DIST) {
        cur += direction*step;
        step = min(cur_scene(cur).closest, length(to - cur));
    }

    return length(to - cur) <= 100*MIN_DIST;
}

float4 RayTrace(float x, float y, vec3 ray_dir, float w, float h) {
    vec3 cur = cam_pos + cam_dir + vec3(x, y, 0);
//    vec3 cur = vec3(x, y, 500);
    vec4 color = backgroundColor;
    while (!isOutOfScene(cur)) {
        Scene scene = cur_scene(cur);
        if (scene.closest <= MIN_DIST) {
            color = scene.color;
            vec3[] ligths = cur_lights();
            for (int i = 0; i < ligths.length(); ++i) {
                vec3 ligth = ligths[i];
                if (isVisible(ligth, cur)) {
                    vec3 normal = estimateNormal(cur);
                    color += max(dot(normalize(ligth - cur), normal), 0.f);
                }
            }
            break;
        }
        cur += scene.closest*ray_dir;
    }
    return color;
}

void main(void) {
  float w = float(g_screenWidth);
  float h = float(g_screenHeight);
  
  // get curr pixelcoordinates
  float x = fragmentTexCoord.x*w - w/2;
  float y = fragmentTexCoord.y*h - h/2;
  
  // generate initial ray
//  float3 ray_pos = float3(0,0,0);
  float3 ray_dir = EyeRayDir(x,y,w,h);
 
  // transorm ray with matrix
  //
//  ray_pos = (g_rayMatrix*float4(ray_pos,1)).xyz;
//    ray_dir = float3x3(g_rayMatrix)*ray_dir;
 
//  if(!RayBoxIntersection(ray_pos, ray_dir, g_bBoxMin, g_bBoxMax, tmin, tmax))
//  {
//    fragColor = g_bgColor;
//    return;
//  }

	
//	float alpha = 1.0f;
//	float3 color = RayMarchConstantFog(tmin, tmax, alpha);
//	fragColor = float4(color,0)*(1.0f-alpha) + g_bgColor*alpha;

	fragColor = RayTrace(x, y, ray_dir, w, h);

//    fragColor = vec4(x/w, y/h, 0, 1);

//    float hui = dot(vec3(0, 0, -1), vec3(ray_pos.x, ray_pos.y, ray_pos.z));
//    fragColor = vec4(hui, hui, hui, 0);

//    if (x < 100 && y < 10) {
//    	fragColor = vec4(1, 1, 0, 0);
//    } else {
//    	fragColor = vec4(1, 0, 0, 0);
//    }
}
