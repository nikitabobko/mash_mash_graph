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

struct Object {
    GLfloat *my_mesh;
    int my_mesh_len;
    GLuint my_vertex_buffer_id = 0;
    vec3 my_position;

    bool my_has_texture;
    GLuint my_texture;

    GLuint my_uv_id = 0;
    GLfloat *my_uv;



//    Object() {
//    }

    Object(GLfloat *mesh, GLsizeiptr mesh_size_in_bytes, glm::vec3 position, bool has_texture, GLuint texture,
            GLfloat *uv, GLsizeiptr uv_size_in_bytes) {
        my_has_texture = has_texture;
        my_texture = texture;
        my_position = position;
        my_mesh_len = mesh_size_in_bytes / sizeof(*mesh);
        my_uv = uv;
//        glm::mat4 model_matrix = glm::rotate(get_model_matrix(), M_PI_4f32, glm::vec3(1, 0, 1));
        my_mesh = mesh;
        glGenBuffers(1, &my_vertex_buffer_id);
        glBindBuffer(GL_ARRAY_BUFFER, my_vertex_buffer_id);
        glBufferData(GL_ARRAY_BUFFER, mesh_size_in_bytes, my_mesh, GL_STATIC_DRAW);

        glGenBuffers(1, &my_uv_id);
        glBindBuffer(GL_ARRAY_BUFFER, my_uv_id);
        glBufferData(GL_ARRAY_BUFFER, uv_size_in_bytes, my_uv, GL_STATIC_DRAW);
    }

    virtual glm::mat4 get_model_matrix(long time_millis) {
        return glm::translate(glm::mat4(1.0f), my_position);
    }

//    Object(const Object& other) = delete;
//    const Object &operator =(const Object &other) = delete;

    void draw(ShaderProgram &program, long time_millis, const glm::mat4 &pv) {
        glm::mat4 PVM = pv * get_model_matrix(time_millis);
        program.SetUniform("PVM", PVM);

        // mesh
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, my_vertex_buffer_id);
        glVertexAttribPointer(MESH_ATTRIB_INDEX, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        // UV
        glEnableVertexAttribArray(UV_ATTRIB_INDEX);
        glBindBuffer(GL_ARRAY_BUFFER, my_uv_id);
        glVertexAttribPointer(UV_ATTRIB_INDEX, 2,GL_FLOAT, GL_FALSE, 0, nullptr);

        glDrawArrays(GL_TRIANGLES, 0, my_mesh_len);

        program.SetUniform("has_texture", my_has_texture);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, my_texture);
        // Set our "myTextureSampler" sampler to use Texture Unit 0 todo
        program.SetUniform("fragment_texture", 0);
    }

    void delete_it() {
        glDeleteBuffers(1, &my_vertex_buffer_id);
    }
};

struct SpinningObject : public Object {
    float my_spin_speed;
    long my_init_time;

    SpinningObject(GLfloat *mesh, GLsizeiptr mesh_size_in_bytes, glm::vec3 position, bool has_texture, GLuint texture,
                   GLfloat *uv, GLsizeiptr uv_size_in_bytes, float spin_speed) :
            Object(mesh, mesh_size_in_bytes, position, has_texture, texture, uv, uv_size_in_bytes) {
        my_spin_speed = spin_speed;
        my_init_time = get_cur_time_millis();
    }

    glm::mat4 get_model_matrix(long time_millis) override {
        float angle = my_spin_speed * (time_millis - my_init_time);
        return glm::rotate(Object::get_model_matrix(time_millis), angle, glm::vec3(1, 1, 1));
    }
};

struct Scene {
    Object **my_objects;
    int my_objects_len;
    glm::mat4 my_projection_matrix = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
    // Camera matrix
    glm::mat4 my_view_matrix = glm::lookAt(
            glm::vec3(3, 4, 10), // Camera pos, in World Space
            glm::vec3(0, 0, 0), // and looks at the origin
            glm::vec3(0, 1, 0)  // Head is up
    );

    Scene(Object **objects, int objects_len) {
        my_objects = objects;
        my_objects_len = objects_len;
    }

    void draw(ShaderProgram &program, long time_millis) {
        glEnableVertexAttribArray(MESH_ATTRIB_INDEX);
        for (int i = 0; i < my_objects_len; ++i) {
            my_objects[i]->draw(program, time_millis, my_projection_matrix * my_view_matrix);
        }
//        glDisableVertexAttribArray(0);
    }

    void delete_it() {
        for (int i = 0; i < my_objects_len; ++i) {
            my_objects[i]->delete_it();
            delete my_objects[i];
        }
    }
};

#endif
