#include "ShaderProgram.h"
//External dependencies
#define GLFW_DLL

static const int MESH_ATTRIB_INDEX = 0;

#include <GLFW/glfw3.h>

#include <memory>
#include <vector>

struct Object {
    GLfloat *my_mesh;
    GLuint my_vertex_buffer_id = 0;

    Object() {
    }

    Object(GLfloat *mesh, GLsizeiptr size_in_bytes) {
        std::cout << "object creation" << std::endl;
        my_mesh = mesh;
        glGenBuffers(1, &my_vertex_buffer_id);
        glBindBuffer(GL_ARRAY_BUFFER, my_vertex_buffer_id);
        glBufferData(GL_ARRAY_BUFFER, size_in_bytes, my_mesh, GL_STATIC_DRAW);
    }

//    Object(const Object& other) = delete;
//    const Object &operator =(const Object &other) = delete;

    void draw() {
        glBindBuffer(GL_ARRAY_BUFFER, my_vertex_buffer_id);
        glVertexAttribPointer(MESH_ATTRIB_INDEX, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    void delete_it() {
        glDeleteBuffers(1, &my_vertex_buffer_id);
    }
};

struct Scene {
    Object *my_objects;
    int my_objects_len;

    Scene(Object *objects, int objects_len) {
        my_objects = objects;
        my_objects_len = objects_len;
    }

    void draw() {
        glEnableVertexAttribArray(MESH_ATTRIB_INDEX);
        for (int i = 0; i < my_objects_len; ++i) {
            my_objects[i].draw();
        }
//        glDisableVertexAttribArray(0);
    }

    void delete_it() {
        for (int i = 0; i < my_objects_len; ++i) {
            my_objects[i].delete_it();
        }
    }
};

