#version 330 core

layout(location = 0) in vec3 triangle;
layout(location = 1) in vec2 vertex_texture_coords;
layout(location = 2) in vec3 vertex_color;

out vec2 texture_coords;
out vec3 fragment_color;

uniform mat4 PVM;

void main() {
    gl_Position = PVM * vec4(triangle, 1);
    texture_coords = vertex_texture_coords;
    fragment_color = vertex_color;
}
