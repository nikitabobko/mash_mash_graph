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
static const int UV_ATTRIB_INDEX = 1;
static const int COLOR_ATTRIB_INDEX = 2;

struct Object {
    GLfloat *my_mesh;
    int my_mesh_len;
    GLuint my_vertex_buffer_id = 0;
    vec3 my_position;

    GLuint my_uv_id = 0;
    GLfloat *my_uv = nullptr;
    GLuint my_texture = 0;

    GLuint my_color_buffer_id = 0;
    GLfloat *my_color_buffer = nullptr;

    float my_bounding_box;
    vec3 my_direction = vec3(0);
    long my_prev_time = 0;

    float my_scale_factor = 1.0f;

    Object() = delete;

    Object(GLfloat *mesh, GLsizeiptr mesh_size_in_bytes, glm::vec3 position) {
        if (position == vec3(0)) {
            position = vec3(integer_rand(global_bound));
        }
        my_position = position;
        my_mesh_len = mesh_size_in_bytes / sizeof(*mesh);

        my_mesh = mesh;
        glGenBuffers(1, &my_vertex_buffer_id);
        glBindBuffer(GL_ARRAY_BUFFER, my_vertex_buffer_id);
        glBufferData(GL_ARRAY_BUFFER, mesh_size_in_bytes, my_mesh, GL_STATIC_DRAW);
    }

    Object *scale(float factor) {
        my_scale_factor = factor;
        return this;
    }

    Object *set_texture(GLuint texture, GLfloat *uv, GLsizeiptr uv_size_in_bytes) {
        my_texture = texture;
        my_uv = uv;
        glGenBuffers(1, &my_uv_id);
        glBindBuffer(GL_ARRAY_BUFFER, my_uv_id);
        glBufferData(GL_ARRAY_BUFFER, uv_size_in_bytes, my_uv, GL_STATIC_DRAW);
        return this;
    }

    Object *set_color(GLfloat *color_buffer, int size_in_bytes) {
        my_color_buffer = color_buffer;
        glGenBuffers(1, &my_color_buffer_id);
        glBindBuffer(GL_ARRAY_BUFFER, my_color_buffer_id);
        glBufferData(GL_ARRAY_BUFFER, size_in_bytes, my_color_buffer, GL_STATIC_DRAW);
        return this;
    }

    Object *add_random_movement() {
        float speeds[] = {0.001f, 0.002f, 0.003f};

        return add_movement(speeds[positive_rand(sizeof(speeds) / sizeof(*speeds))],
                vec3(integer_rand(1000), integer_rand(1000), integer_rand(1000)),
                global_bound);
    }

    Object *add_movement(float speed, vec3 direction, float bounding_box) {
        my_bounding_box = bounding_box;
        my_direction = speed * glm::normalize(direction);
        my_prev_time = get_cur_time_millis();
        return this;
    }

    vec3 calc_current_position(long time_millis) {
        return my_position + float(time_millis - my_prev_time) * my_direction;
    }

    virtual glm::mat4 get_model_matrix(long time_millis) {
        my_position = calc_current_position(time_millis);
        my_prev_time = time_millis;
        if (glm::length(my_direction) > 0.0000001 &&
            (std::abs(my_position.x) > my_bounding_box ||
             std::abs(my_position.y) > my_bounding_box ||
             std::abs(my_position.z) > my_bounding_box)) {
            vec3 normal = glm::normalize(vec3(-1. * (std::abs(my_position.x) > my_bounding_box) * my_position.x,
                                              -1. * (std::abs(my_position.y) > my_bounding_box) * my_position.y,
                                              -1. * (std::abs(my_position.z) > my_bounding_box) * my_position.z));
            float len = glm::length(my_direction);
            my_direction = 2 * glm::dot(normal, -my_direction) * normal + my_direction;
            my_direction -= 0.00001f * glm::normalize(my_position);
            my_direction = len * glm::normalize(my_direction);
        }
        return glm::translate(glm::mat4(1.0f), my_position) * glm::scale(glm::mat4(1.0f), vec3(my_scale_factor));
    }

    Object(const Object& other) = delete;
    const Object &operator =(const Object &other) = delete;

    void draw(ShaderProgram &program, long time_millis, const glm::mat4 &pv) {
        glm::mat4 PVM = pv * get_model_matrix(time_millis);
        program.SetUniform("PVM", PVM);

        // mesh
        glEnableVertexAttribArray(MESH_ATTRIB_INDEX);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, my_vertex_buffer_id);
        glVertexAttribPointer(MESH_ATTRIB_INDEX, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        // UV
        program.SetUniform("has_texture", my_uv != nullptr);
        if (my_uv != nullptr) {
            glEnableVertexAttribArray(UV_ATTRIB_INDEX);
            glBindBuffer(GL_ARRAY_BUFFER, my_uv_id);
            glVertexAttribPointer(UV_ATTRIB_INDEX, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, my_texture);
            // Set our "myTextureSampler" sampler to use Texture Unit 0 todo
            program.SetUniform("fragment_texture", 0);
        }

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
        if (my_uv != nullptr) {
            glDisableVertexAttribArray(UV_ATTRIB_INDEX);
        }
    }

    void delete_it() {
        glDeleteBuffers(1, &my_vertex_buffer_id);
        if (my_uv != nullptr) {
            glDeleteBuffers(1, &my_uv_id);
        }
        if (my_color_buffer != nullptr) {
            glDeleteBuffers(1, &my_color_buffer_id);
        }
    }
};

struct SpinningObject : public Object {
    float my_spin_speed;
    long my_spinning_init_time;
    vec3 cur_rot_vec = vec3(1, 1, 1);
    mat4 cur_rot_matrix = mat4(1);

//    SpinningObject(GLfloat *mesh, GLsizeiptr mesh_size_in_bytes, glm::vec3 position, float spin_speed) :
//            Object(mesh, mesh_size_in_bytes, position) {
//        my_spin_speed = spin_speed;
//        my_spinning_init_time = get_cur_time_millis();
//    }

    SpinningObject(GLfloat *mesh, GLsizeiptr mesh_size_in_bytes, glm::vec3 position) :
            Object(mesh, mesh_size_in_bytes, position) {
        float speeds[] = {0.001f, 0.002f, 0.003f};
        my_spin_speed = speeds[positive_rand(sizeof(speeds)/ sizeof(*speeds))];
        my_spinning_init_time = get_cur_time_millis();
    }

    glm::mat4 get_model_matrix(long time_millis) override {
        float speeds[] = {0.001f, 0.002f, 0.003f};
        float angle = my_spin_speed * (time_millis - my_spinning_init_time);
        mat4 now_rotate = glm::rotate(mat4(1), angle, cur_rot_vec) * cur_rot_matrix;
        if (positive_rand(100) == 0) { // todo uncomment
            my_spin_speed = speeds[positive_rand(sizeof(speeds)/ sizeof(*speeds))];
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
    // Camera matrix
    glm::mat4 my_view_matrix = glm::lookAt(
            glm::vec3(0, 0, 17), // Camera pos, in World Space
            glm::vec3(0, 0, 0), // and looks at the origin
            glm::vec3(0, 1, 0)  // Head is up
    );
    Object *fixate_object = nullptr;

    Scene(Object **objects, int objects_len) {
        my_objects = objects;
        my_objects_len = objects_len;
    }

    void fixate_camera_on_random_object() {
        fixate_object = my_objects[positive_rand(my_objects_len)];
    }

    void undo_fixate_camera() {
        fixate_object = nullptr;
    }

    void draw(ShaderProgram &program, long time_millis) {
        mat4 view_matrix = my_view_matrix;
        if (fixate_object != nullptr) {
            view_matrix = glm::translate(view_matrix, vec3(0, 0, 10)-(fixate_object->calc_current_position(time_millis)));
        }
        for (int i = 0; i < my_objects_len; ++i) {
            my_objects[i]->draw(program, time_millis, my_projection_matrix * view_matrix);
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
