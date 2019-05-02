#ifndef CORE_H
#define CORE_H

#include "ShaderProgram.h"
#include "util.h"

#define GLFW_DLL

#include <chrono>
#include <GLFW/glfw3.h>
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

static const int MESH_ATTRIB_INDEX = 0;
static const int TEXTURE_ATTRIB_INDEX = 1;
static const int COLOR_ATTRIB_INDEX = 2;
static const int NORMAL_ATTRIB_INDEX = 3;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float specular_shiness;

    Material() {

    }

    Material(const vec3 &ambient, const vec3 &diffuse, const vec3 &specular, float specularShiness) :
            ambient(ambient), diffuse(diffuse), specular(specular), specular_shiness(specularShiness) {
    }
};

struct Object {
    GLfloat *my_mesh;
    int my_mesh_len;
    GLuint my_vertex_buffer_id = 0;
    vec3 my_position;

    GLuint my_normal_buffer_id = 0;

    GLuint my_texture_id = 0;
    GLfloat *my_texture_coords = nullptr;
    GLuint my_texture_res = 0;

    GLuint my_color_buffer_id = 0;
    GLfloat *my_color_buffer = nullptr;

    float my_scale_factor = 1.0f;

    Material my_material;

    Object() = delete;

    Object(GLfloat *mesh, GLsizeiptr mesh_size_in_bytes, glm::vec3 position, GLfloat *normals,
           GLsizeiptr normals_size_in_bytes, const Material &material) {
        my_material = material;
        my_position = position;
        my_mesh_len = mesh_size_in_bytes / sizeof(*mesh);

        my_mesh = mesh;
        glGenBuffers(1, &my_vertex_buffer_id);
        glBindBuffer(GL_ARRAY_BUFFER, my_vertex_buffer_id);
        glBufferData(GL_ARRAY_BUFFER, mesh_size_in_bytes, my_mesh, GL_STATIC_DRAW);

        glGenBuffers(1, &my_normal_buffer_id);
        glBindBuffer(GL_ARRAY_BUFFER, my_normal_buffer_id);
        glBufferData(GL_ARRAY_BUFFER, normals_size_in_bytes, normals, GL_STATIC_DRAW);
    }

    Object *scale(float factor) {
        my_scale_factor = factor;
        return this;
    }

    Object *set_texture(GLuint texture, GLfloat *texture_coords, GLsizeiptr texture_coords_size_in_bytes) {
        my_texture_res = texture;
        my_texture_coords = texture_coords;
        glGenBuffers(1, &my_texture_id);
        glBindBuffer(GL_ARRAY_BUFFER, my_texture_id);
        glBufferData(GL_ARRAY_BUFFER, texture_coords_size_in_bytes, my_texture_coords, GL_STATIC_DRAW);
        return this;
    }

    Object *set_color(GLfloat *color_buffer, int size_in_bytes) {
        my_color_buffer = color_buffer;
        glGenBuffers(1, &my_color_buffer_id);
        glBindBuffer(GL_ARRAY_BUFFER, my_color_buffer_id);
        glBufferData(GL_ARRAY_BUFFER, size_in_bytes, my_color_buffer, GL_STATIC_DRAW);
        return this;
    }

    virtual glm::mat4 get_model_matrix(long time_millis) {
        return glm::translate(glm::mat4(1.0f), my_position) * glm::scale(glm::mat4(1.0f), vec3(my_scale_factor));
    }

    Object(const Object &other) = delete;

    const Object &operator=(const Object &other) = delete;

    void draw(ShaderProgram &program, long time_millis, const glm::mat4 &projection, const glm::mat4 &view,
              const vec3 &light_pos) {
        const mat4 &model_matrix = get_model_matrix(time_millis);
        const mat4 &VM = view * model_matrix;
        program.SetUniform("PVM", projection * VM);
        program.SetUniform("VM", VM);

        program.SetUniform("ambient", my_material.ambient);
        program.SetUniform("diffuse", my_material.diffuse);
        program.SetUniform("specular", my_material.specular);
        program.SetUniform("specular_shiness", my_material.specular_shiness);

        program.SetUniform("light_position", light_pos);

        // mesh
        glEnableVertexAttribArray(MESH_ATTRIB_INDEX);
        glBindBuffer(GL_ARRAY_BUFFER, my_vertex_buffer_id);
        glVertexAttribPointer(MESH_ATTRIB_INDEX, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        // Normal
        glEnableVertexAttribArray(NORMAL_ATTRIB_INDEX);
        glBindBuffer(GL_ARRAY_BUFFER, my_normal_buffer_id);
        glVertexAttribPointer(NORMAL_ATTRIB_INDEX, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        // Texture
        program.SetUniform("has_texture", my_texture_coords != nullptr);
        if (my_texture_coords != nullptr) {
            glEnableVertexAttribArray(TEXTURE_ATTRIB_INDEX);
            glBindBuffer(GL_ARRAY_BUFFER, my_texture_id);
            glVertexAttribPointer(TEXTURE_ATTRIB_INDEX, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, my_texture_res);
            program.SetUniform("fragment_texture", 0);
        }

        // Color
        program.SetUniform("has_color", my_color_buffer != nullptr);
        if (my_color_buffer != nullptr) {
            glEnableVertexAttribArray(COLOR_ATTRIB_INDEX);
            glBindBuffer(GL_ARRAY_BUFFER, my_color_buffer_id);
            glVertexAttribPointer(COLOR_ATTRIB_INDEX, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        }

        glDrawArrays(GL_TRIANGLES, 0, my_mesh_len);

        if (my_color_buffer != nullptr) {
            glDisableVertexAttribArray(COLOR_ATTRIB_INDEX);
        }
        if (my_texture_coords != nullptr) {
            glDisableVertexAttribArray(TEXTURE_ATTRIB_INDEX);
        }
    }

    void delete_it() {
        glDeleteBuffers(1, &my_vertex_buffer_id);
        if (my_texture_coords != nullptr) {
            glDeleteBuffers(1, &my_texture_id);
        }
        if (my_color_buffer != nullptr) {
            glDeleteBuffers(1, &my_color_buffer_id);
        }
    }
};

struct SwingingObject : public Object {
    float my_speed;
    float my_amplitude;
    long my_swinging_init_time;

    SwingingObject(GLfloat *mesh, GLsizeiptr meshSizeInBytes, const vec3 &position, GLfloat *normals,
                   GLsizeiptr normalsSizeInBytes, const Material &material, float mySpeed, float amplitude) :
            Object(mesh, meshSizeInBytes, position, normals, normalsSizeInBytes, material) {
        my_speed = mySpeed;
        my_amplitude = amplitude;
        my_swinging_init_time = get_cur_time_millis();
    }

    glm::mat4 get_model_matrix(long time_millis) override {
        float angle = my_amplitude * cos(my_speed * (time_millis - my_swinging_init_time));
        return Object::get_model_matrix(time_millis) * glm::rotate(mat4(1), angle, vec3(0, 0, 1));
    }
};

struct SpinningObject : public Object {
    float my_spin_speed;
    long my_spinning_init_time;
    vec3 cur_rot_vec = vec3(1, 1, 1);
    mat4 cur_rot_matrix = mat4(1);

    SpinningObject(GLfloat *mesh, GLsizeiptr mesh_size_in_bytes, glm::vec3 position, GLfloat *normals,
                   GLsizeiptr normals_size_in_bytes, const Material &material) :
            Object(mesh, mesh_size_in_bytes, position, normals, normals_size_in_bytes, material) {
        float speeds[] = {0.001f, 0.002f, 0.003f};
        my_spin_speed = speeds[positive_rand(sizeof(speeds) / sizeof(*speeds))];
        my_spinning_init_time = get_cur_time_millis();
    }

    glm::mat4 get_model_matrix(long time_millis) override {
        float speeds[] = {0.001f, 0.002f, 0.003f};
        float angle = my_spin_speed * (time_millis - my_spinning_init_time);
        mat4 now_rotate = glm::rotate(mat4(1), angle, cur_rot_vec) * cur_rot_matrix;
        if (positive_rand(100) == 0) {
            my_spin_speed = speeds[positive_rand(sizeof(speeds) / sizeof(*speeds))];
            cur_rot_matrix = now_rotate;
            do {
                cur_rot_vec = glm::vec3(integer_rand(2), integer_rand(2), integer_rand(2));
            } while (cur_rot_vec == vec3(0));
            my_spinning_init_time = time_millis;
        }
        return Object::get_model_matrix(time_millis) * now_rotate;
    }
};

struct Scene {
    Object **my_objects;
    int my_objects_len;
    glm::mat4 my_projection_matrix = glm::perspective(glm::radians(45.0f), 1280.0f / 1024.0f, 0.1f, 100.0f);
    glm::vec3 camera_pos = glm::vec3(10, 4, 17);
    // Camera matrix
    glm::mat4 my_view_matrix = glm::lookAt(
            camera_pos, // Camera pos, in World Space
            glm::vec3(0, 0, 0), // and looks at the origin
            glm::vec3(0, 1, 0)  // Head is up
    );
    glm::vec3 light_position = vec3(5, 0, 0);

    Scene(Object **objects, int objects_len) {
        my_objects = objects;
        my_objects_len = objects_len;
    }

    void draw(ShaderProgram &program, long time_millis) {
        program.SetUniform("camera_pos", camera_pos);
        for (int i = 0; i < my_objects_len; ++i) {
            my_objects[i]->draw(program, time_millis, my_projection_matrix, my_view_matrix, light_position);
        }
    }

    void delete_it() {
        for (int i = 0; i < my_objects_len; ++i) {
            my_objects[i]->delete_it();
            delete my_objects[i];
        }
    }
};

#endif
