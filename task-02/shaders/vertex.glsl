#version 330 core

layout(location = 0) in vec3 triangle;
layout(location = 1) in vec2 vertex_UV;

out vec2 UV;

uniform mat4 PVM;

void main() {
    gl_Position = PVM * vec4(triangle, 1);
    UV = vertex_UV;
}
