#version 330 core

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec2 vertex_texture_coords;
layout(location = 2) in vec3 vertex_color;
layout(location = 3) in vec3 vertex_normal;

out vec2 texture_coords;
out vec3 fragment_color;
out vec3 fragment_normal;
out vec3 vertex_coords;

uniform mat4 PVM;
uniform mat4 VM;

void main() {
    gl_Position = PVM * vec4(vertex, 1);
    vertex_coords = (VM * vec4(vertex, 1)).xyz;
    texture_coords = vertex_texture_coords;
    fragment_color = vertex_color;
    fragment_normal = (VM * vec4(vertex_normal, 0)).xyz;
}
