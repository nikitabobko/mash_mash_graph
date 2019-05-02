#version 330 core

out vec3 color;

in vec3 vertex_coords;
in vec2 texture_coords;
in vec3 fragment_color;
in vec3 fragment_normal;

uniform vec3 light_position;
uniform sampler2D fragment_texture;
uniform int has_texture;
uniform int has_color;
uniform vec3 camera_pos;

uniform vec3 ambient;
uniform vec3 diffuse;
uniform vec3 specular;
uniform float specular_shiness;

void main() {
    vec3 local_color;
    if (has_texture == 1) {
        local_color = texture(fragment_texture, texture_coords).rgb;
    } else if (has_color == 1) {
        local_color = fragment_color;
    } else {
        local_color = vec3(0.0f, 0.0f, 0.0f);
    }
    color = local_color;

    vec3 ray_dir = normalize(vertex_coords - camera_pos);
    vec3 to_light = normalize(light_position - vertex_coords);
    float scalar = max(dot(fragment_normal, to_light), 0);
    vec3 reflected_light = 2*scalar*fragment_normal - to_light;

    color += ambient;
    color += diffuse*scalar;
    color += specular*pow(max(dot(-ray_dir, reflected_light), 0.f), 128*specular_shiness);
}
