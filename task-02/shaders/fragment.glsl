#version 330 core

out vec3 color;

in vec2 texture_coords;
in vec3 fragment_color;

uniform sampler2D fragment_texture;
uniform int has_texture;
uniform int has_color;

void main() {
    vec3 local_color;
    if (has_texture == 1) {
        local_color = texture(fragment_texture, texture_coords).rgb;
    } else if (has_color == 1) {
        local_color = fragment_color;
    } else {
        local_color = vec3(1.0f, 0.0f, 0.0f);
    }
    color = local_color;
}
