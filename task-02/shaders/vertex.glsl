#version 330 core

layout(location = 0) in vec3 triangle;

uniform mat4 PVM;

void main() {
    gl_Position = PVM * vec4(triangle, 1);
}
