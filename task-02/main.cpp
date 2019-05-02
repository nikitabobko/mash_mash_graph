#include "common.h"
#include "core.cpp"
#include "ShaderProgram.h"
#include "util.h"

#define GLFW_DLL

#include <GLFW/glfw3.h>
#include <random>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <dirent.h>
#include "SOIL.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace glm;

static const GLsizei WIDTH = 1280, HEIGHT = 1024; //размеры окна

GLuint load_texture(const char *filename) {
    int width, height;
    unsigned char *image = SOIL_load_image(filename, &width, &height, 0, SOIL_LOAD_RGB);

    if (image == nullptr) {
        std::cerr << "Cannot load image " << filename << std::endl;
        return 0;
    }

    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

    SOIL_free_image_data(image);
    glGenerateMipmap(GL_TEXTURE_2D);
    return texture_id;
}

int initGL() {
    int res = 0;
    //грузим функции opengl через glad
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }

    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    return 0;
}

void assimp_loaded_obj_to_internal(const aiScene *loaded_obj, std::vector<GLfloat> *mesh,
                                   std::vector<GLfloat> *texture_coords) {
    for (int mesh_index = 0; mesh_index < loaded_obj->mNumMeshes; ++mesh_index) {
        aiMesh *cur_mesh = loaded_obj->mMeshes[mesh_index];
        for (int i = 0; i < cur_mesh->mNumVertices; ++i) {
            mesh->push_back(cur_mesh->mVertices[i].x);
            mesh->push_back(cur_mesh->mVertices[i].y);
            mesh->push_back(cur_mesh->mVertices[i].z);
        }

        if (cur_mesh->mTextureCoords[0] != nullptr) {
            for (int i = 0; i < cur_mesh->mNumVertices; ++i) {
                texture_coords->push_back(cur_mesh->mTextureCoords[0][i].x);
                texture_coords->push_back(cur_mesh->mTextureCoords[0][i].y);
            }
        }
    }
}

bool fixate = false;

void on_key_click(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F2 && action == GLFW_PRESS) {
        fixate = true;
    }

    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        fixate = false;
    }
}

int main(int argc, char **argv) {
    if (!glfwInit())
        return -1;

    //запрашиваем контекст opengl версии 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL basic sample", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    glfwSetKeyCallback(window, on_key_click);

    if (initGL() != 0)
        return -1;

    //Reset any OpenGL errors which could be present for some reason
    GLenum gl_error = glGetError();
    while (gl_error != GL_NO_ERROR)
        gl_error = glGetError();

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    std::unordered_map<GLenum, std::string> shaders;
    shaders[GL_VERTEX_SHADER] = "vertex.glsl";
    shaders[GL_FRAGMENT_SHADER] = "fragment.glsl";
    ShaderProgram program(shaders);
    GL_CHECK_ERRORS;

    glfwSwapInterval(1); // force 60 frames per second

    // magic
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint camo_texture = load_texture("res/camo.bmp");

    GLuint metalroof_texture = load_texture("res/metalroof1.bmp");

    GLfloat cube_mesh[] = {
            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, +1.0f,
            -1.0f, +1.0f, +1.0f,
            +1.0f, +1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f, +1.0f, -1.0f,
            +1.0f, -1.0f, +1.0f,
            -1.0f, -1.0f, -1.0f,
            +1.0f, -1.0f, -1.0f,
            +1.0f, +1.0f, -1.0f,
            +1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f, +1.0f, +1.0f,
            -1.0f, +1.0f, -1.0f,
            +1.0f, -1.0f, +1.0f,
            -1.0f, -1.0f, +1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f, +1.0f, +1.0f,
            -1.0f, -1.0f, +1.0f,
            +1.0f, -1.0f, +1.0f,
            +1.0f, +1.0f, +1.0f,
            +1.0f, -1.0f, -1.0f,
            +1.0f, +1.0f, -1.0f,
            +1.0f, -1.0f, -1.0f,
            +1.0f, +1.0f, +1.0f,
            +1.0f, -1.0f, +1.0f,
            +1.0f, +1.0f, +1.0f,
            +1.0f, +1.0f, -1.0f,
            -1.0f, +1.0f, -1.0f,
            +1.0f, +1.0f, +1.0f,
            -1.0f, +1.0f, -1.0f,
            -1.0f, +1.0f, +1.0f,
            +1.0f, +1.0f, +1.0f,
            -1.0f, +1.0f, +1.0f,
            +1.0f, -1.0f, +1.0f
    };

    GLfloat cube_color[] = {
            1.0f, 0.0f, 0.0f, // red
            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, // blue
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,
            1.0f, 1.0f, 0.0f, // yellow
            1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 1.0f, // blue
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 0.0f, // red
            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            1.0f, 1.0f, 0.0f, // yellow
            1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            1.0f, 0.0f, 1.0f, // purple
            1.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 1.0f,
            0.0f, 1.0f, 1.0f, // cyan
            0.0f, 1.0f, 1.0f,
            0.0f, 1.0f, 1.0f,
            0.0f, 1.0f, 1.0f, // cyan
            0.0f, 1.0f, 1.0f,
            0.0f, 1.0f, 1.0f,
            0.0f, 1.0f, 0.0f, // green
            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f, // green
            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            1.0f, 0.0f, 1.0f, // purple
            1.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 1.0f,
    };

    GLfloat cube_texture_coords[] = {
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            1.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,
            0.0f, 0.0f,
            0.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,
            0.0f, 0.0f,
            0.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 0.0f,
            0.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,
            1.0f, 0.0f,
    };

    GLfloat triangle_mesh[] = {
            -1.0f, -1.0f, +0.0f,
            +1.0f, -1.0f, +0.0f,
            +0.0f, +1.0f, +0.0f,
    };

    GLfloat triangle_color[] = {
            1.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 1.0f,
    };

    GLfloat plane_scale = 30;
    GLfloat back_plane_mesh[] = {
            -plane_scale, -plane_scale, 0,
            -plane_scale, plane_scale, 0,
            plane_scale, -plane_scale, 0,

            plane_scale, plane_scale, 0,
            plane_scale, -plane_scale, 0,
            -plane_scale, plane_scale, 0
    };

    GLfloat back_plane_texture_coords[] = {
            -1, -1,
            -1, 1,
            1, -1,

            1, 1,
            1, -1,
            -1, 1
    };

    GLfloat tetrahedron_mesh[] = {
            -1.0f, -1.0f, +1.0f,
            +1.0f, -1.0f, +1.0f,
            +0.0f, +1.0f, +1.0f,

            -1.0f, -1.0f, +1.0f,
            +1.0f, -1.0f, +1.0f,
            +0.0f, +0.0f, -1.0f,

            +1.0f, -1.0f, +1.0f,
            +0.0f, +1.0f, +1.0f,
            +0.0f, +0.0f, -1.0f,

            -1.0f, -1.0f, +1.0f,
            +0.0f, +1.0f, +1.0f,
            +0.0f, +0.0f, -1.0f,
    };

    GLfloat tetrahedron_color[] = {
            0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f,

            1.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 1.0f,

            1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,

            0.5f, 0.5f, 0.5f,
            0.5f, 0.5f, 0.5f,
            0.5f, 0.5f, 0.5f,
    };

    Assimp::Importer importer;
    const aiScene *loaded_obj = importer.ReadFile("res/obj/thor_hammer.obj", aiProcess_Triangulate);

    std::vector<GLfloat> hammer_mesh;
    std::vector<GLfloat> hammer_texture_coords;
    assimp_loaded_obj_to_internal(loaded_obj, &hammer_mesh, &hammer_texture_coords);

    srand(time(0));

    GLuint hammer_texture = load_texture("res/obj/hammer_texture.jpg");

    Object *objects[] = {
            (new Object(back_plane_mesh, sizeof(back_plane_mesh),
                        vec3(-plane_scale / 2, 0, -global_bound - plane_scale)))
                    ->set_texture(metalroof_texture, back_plane_texture_coords, sizeof(back_plane_texture_coords)),
            (new Object(back_plane_mesh, sizeof(back_plane_mesh),
                        vec3(plane_scale / 2, 0, -global_bound - plane_scale)))
                    ->set_texture(metalroof_texture, back_plane_texture_coords, sizeof(back_plane_texture_coords)),

            (new SpinningObject(cube_mesh, sizeof(cube_mesh), vec3(0)))
                    ->set_texture(camo_texture, cube_texture_coords, sizeof(cube_texture_coords))
                    ->add_random_movement(),
            (new SpinningObject(triangle_mesh, sizeof(triangle_mesh), vec3(0)))
                    ->set_color(triangle_color, sizeof(triangle_color))
                    ->add_random_movement(),
            (new SpinningObject(tetrahedron_mesh, sizeof(tetrahedron_mesh), vec3(0)))
                    ->set_color(tetrahedron_color, sizeof(tetrahedron_color))
                    ->add_random_movement(),
            (new SpinningObject(cube_mesh, sizeof(cube_mesh), vec3(0)))
                    ->add_random_movement()
                    ->set_color(cube_color, sizeof(cube_color)),
            (new SpinningObject(hammer_mesh.data(), hammer_mesh.size() * sizeof(hammer_mesh[0]), vec3(0)))
                    ->add_random_movement()
                    ->set_texture(hammer_texture, hammer_texture_coords.data(),
                                  hammer_texture_coords.size() * sizeof(hammer_texture_coords[0]))
                    ->scale(0.8),

            (new SpinningObject(cube_mesh, sizeof(cube_mesh), vec3(0)))
                    ->set_texture(camo_texture, cube_texture_coords, sizeof(cube_texture_coords))
                    ->add_random_movement(),
            (new SpinningObject(triangle_mesh, sizeof(triangle_mesh), vec3(0)))
                    ->set_color(triangle_color, sizeof(triangle_color))
                    ->add_random_movement(),
            (new SpinningObject(tetrahedron_mesh, sizeof(tetrahedron_mesh), vec3(0)))
                    ->set_color(tetrahedron_color, sizeof(tetrahedron_color))
                    ->add_random_movement(),
            (new SpinningObject(cube_mesh, sizeof(cube_mesh), vec3(0)))
                    ->add_random_movement()
                    ->set_color(cube_color, sizeof(cube_color)),
            (new SpinningObject(cube_mesh, sizeof(cube_mesh), vec3(0)))
                    ->add_random_movement()
                    ->set_color(cube_color, sizeof(cube_color)),
    };
    Scene scene(objects, sizeof(objects) / sizeof(*objects));

    // main loop
    while (!glfwWindowShouldClose(window)) {

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (fixate && scene.fixate_object == nullptr) {
            do {
                scene.fixate_camera_on_random_object();
            } while (scene.fixate_object->my_mesh == back_plane_mesh);
        }

        if (!fixate) {
            scene.undo_fixate_camera();
        }

        program.StartUseShader();

        scene.draw(program, get_cur_time_millis());

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    scene.delete_it();

    // Close OpenGL window and terminate GLFW
    glfwTerminate();
    glDeleteVertexArrays(1, &vao);

    return 0;
}

// todo check all comments. Read entire code once more
// todo Check on Ubuntu virtual machine
