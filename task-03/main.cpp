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

static const Material RED_PLASTIC(vec3(0.0, 0.0, 0.0), vec3(0.5, 0.0, 0.0), vec3(0.7, 0.6, 0.6), 0.25);
static const Material NO_MATERIAL(vec3(0.0), vec3(0), vec3(0), 1);
static const Material GREEN_RUBER(vec3(0.0, 0.05, 0.0), vec3(0.4, 0.5, 0.4), vec3(0.04, 0.7, 0.04), .078125);
static const Material BLACK_PLASTIC(vec3(0.0, 0.0, 0.0), vec3(0.01, 0.01, 0.01), vec3(0.50, 0.50, 0.50), 0.25f);
static const Material COPPER(vec3(0.19125, 0.0735, 0.0225), vec3(0.7038, 0.27048, 0.0828),
                             vec3(0.256777, 0.137622, 0.086014), 0.1);
static const Material SILVER_MATERIAL(vec3(0.19225, 0.19225, 0.19225), vec3(0.50754, 0.50754, 0.50754),
                                      vec3(0.508273, 0.508273, 0.508273), 0.4);

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

void assimp_loaded_obj_to_internal(const aiScene *loaded_obj,
                                   std::vector<GLfloat> *mesh,
                                   std::vector<GLfloat> *texture_coords,
                                   std::vector<GLfloat> *normals) {
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

        for (int i = 0; i < cur_mesh->mNumVertices; ++i) {
            normals->push_back(cur_mesh->mNormals[i].x);
            normals->push_back(cur_mesh->mNormals[i].y);
            normals->push_back(cur_mesh->mNormals[i].z);
        }
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

    if (initGL() != 0)
        return -1;

    //Reset any OpenGL errors which could be present for some reason
    GLenum gl_error = glGetError();
    while (gl_error != GL_NO_ERROR)
        gl_error = glGetError();

    glEnable(GL_DEPTH_TEST);
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

    GLfloat cube_mesh[] = {
            -1.0f, -1.0f, -1.0f, // 1
            -1.0f, -1.0f, +1.0f,
            -1.0f, +1.0f, +1.0f,
            -1.0f, -1.0f, -1.0f, // 5
            -1.0f, +1.0f, +1.0f,
            -1.0f, +1.0f, -1.0f,

            +1.0f, +1.0f, -1.0f, // 2
            -1.0f, -1.0f, -1.0f,
            -1.0f, +1.0f, -1.0f,
            +1.0f, +1.0f, -1.0f, // 4
            +1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,

            +1.0f, -1.0f, +1.0f, // 3
            -1.0f, -1.0f, -1.0f,
            +1.0f, -1.0f, -1.0f,
            +1.0f, -1.0f, +1.0f, // 6
            -1.0f, -1.0f, +1.0f,
            -1.0f, -1.0f, -1.0f,

            -1.0f, +1.0f, +1.0f, // 7
            -1.0f, -1.0f, +1.0f,
            +1.0f, -1.0f, +1.0f,
            +1.0f, +1.0f, +1.0f, // 12
            -1.0f, +1.0f, +1.0f,
            +1.0f, -1.0f, +1.0f,

            +1.0f, +1.0f, +1.0f, // 8
            +1.0f, -1.0f, -1.0f,
            +1.0f, +1.0f, -1.0f,
            +1.0f, -1.0f, -1.0f, // 9
            +1.0f, +1.0f, +1.0f,
            +1.0f, -1.0f, +1.0f,

            +1.0f, +1.0f, +1.0f, // 10
            +1.0f, +1.0f, -1.0f,
            -1.0f, +1.0f, -1.0f,
            +1.0f, +1.0f, +1.0f, // 11
            -1.0f, +1.0f, -1.0f,
            -1.0f, +1.0f, +1.0f,
    };

    GLfloat cube_normals[] = {
            -1.0f, +0.0f, +0.0f, // 1
            -1.0f, +0.0f, +0.0f,
            -1.0f, +0.0f, +0.0f,
            -1.0f, +0.0f, +0.0f, // 5
            -1.0f, +0.0f, +0.0f,
            -1.0f, +0.0f, +0.0f,

            +0.0f, +0.0f, -1.0f, // 2
            +0.0f, +0.0f, -1.0f,
            +0.0f, +0.0f, -1.0f,
            +0.0f, +0.0f, -1.0f, // 4
            +0.0f, +0.0f, -1.0f,
            +0.0f, +0.0f, -1.0f,

            +0.0f, -1.0f, +0.0f, // 3
            +0.0f, -1.0f, +0.0f,
            +0.0f, -1.0f, +0.0f,
            +0.0f, -1.0f, +0.0f, // 6
            +0.0f, -1.0f, +0.0f,
            +0.0f, -1.0f, +0.0f,

            +0.0f, +0.0f, +1.0f, // 7
            +0.0f, +0.0f, +1.0f,
            +0.0f, +0.0f, +1.0f,
            +0.0f, +0.0f, +1.0f, // 12
            +0.0f, +0.0f, +1.0f,
            +0.0f, +0.0f, +1.0f,

            +1.0f, +0.0f, +0.0f, // 8
            +1.0f, +0.0f, +0.0f,
            +1.0f, +0.0f, +0.0f,
            +1.0f, +0.0f, +0.0f, // 9
            +1.0f, +0.0f, +0.0f,
            +1.0f, +0.0f, +0.0f,

            +0.0f, +1.0f, +0.0f, // 10
            +0.0f, +1.0f, +0.0f,
            +0.0f, +1.0f, +0.0f,
            +0.0f, +1.0f, +0.0f, // 11
            +0.0f, +1.0f, +0.0f,
            +0.0f, +1.0f, +0.0f,
    };

    GLfloat cube_different_color[] = {
            1.0f, 0.0f, 0.0f, // 1 red
            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f, // 5 red
            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,

            0.0f, 0.0f, 1.0f, // 2 blue
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f, // 4 blue
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,

            1.0f, 1.0f, 0.0f, // 3 yellow
            1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 0.0f, // 6 yellow
            1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,

            1.0f, 0.0f, 1.0f, // 7 purple
            1.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 1.0f, // 12 purple
            1.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 1.0f,

            0.0f, 1.0f, 1.0f, // 8 cyan
            0.0f, 1.0f, 1.0f,
            0.0f, 1.0f, 1.0f,
            0.0f, 1.0f, 1.0f, // 9 cyan
            0.0f, 1.0f, 1.0f,
            0.0f, 1.0f, 1.0f,

            0.0f, 1.0f, 0.0f, // 10 green
            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f, // 11 green
            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
    };

    GLfloat cube_purple_color[] = {
            0.0f, 0.0f, 0.6f,
            0.0f, 0.0f, 0.6f,
            0.0f, 0.0f, 0.6f,
            0.0f, 0.0f, 0.6f,
            0.0f, 0.0f, 0.6f,
            0.0f, 0.0f, 0.6f,
            0.0f, 0.0f, 0.6f,
            0.0f, 0.0f, 0.6f,
            0.0f, 0.0f, 0.6f,
            0.0f, 0.0f, 0.6f,
            0.0f, 0.0f, 0.6f,
            0.0f, 0.0f, 0.6f,
            0.0f, 0.0f, 0.6f,
            0.0f, 0.0f, 0.6f,
            0.0f, 0.0f, 0.6f,
            0.0f, 0.0f, 0.6f,
            0.0f, 0.0f, 0.6f,
            0.0f, 0.0f, 0.6f,
            0.0f, 0.0f, 0.6f,
            0.0f, 0.0f, 0.6f,
            0.0f, 0.0f, 0.6f,
            0.0f, 0.0f, 0.6f,
            0.0f, 0.0f, 0.6f,
            0.0f, 0.0f, 0.6f,
            0.0f, 0.0f, 0.6f,
            0.0f, 0.0f, 0.6f,
            0.0f, 0.0f, 0.6f,
            0.0f, 0.0f, 0.6f,
            0.0f, 0.0f, 0.6f,
            0.0f, 0.0f, 0.6f,
            0.0f, 0.0f, 0.6f,
            0.0f, 0.0f, 0.6f,
            0.0f, 0.0f, 0.6f,
            0.0f, 0.0f, 0.6f,
            0.0f, 0.0f, 0.6f,
            0.0f, 0.0f, 0.6f,
    };

    float down_plane_scale = 7.0f;
    GLfloat down_plane_mesh[] = {
            +down_plane_scale, 0, +down_plane_scale,
            +down_plane_scale, 0, -down_plane_scale,
            -down_plane_scale, 0, +down_plane_scale,

            +down_plane_scale, 0, -down_plane_scale,
            -down_plane_scale, 0, +down_plane_scale,
            -down_plane_scale, 0, -down_plane_scale,
    };

    GLfloat down_plane_texture_coords[] = {
            1, 0,
            1, 1,
            0, 0,

            1, 1,
            0, 0,
            0, 1,
    };

    GLfloat down_plane_normals[] = {
            0, 1, 0,
            0, 1, 0,
            0, 1, 0,

            0, 1, 0,
            0, 1, 0,
            0, 1, 0,
    };

    GLuint grass_texture = load_texture("res/grass.jpg");

    Assimp::Importer importer;
    const aiScene *hammer_loaded_obj = importer.ReadFile("res/obj/thor_hammer.obj", aiProcess_Triangulate);

    std::vector<GLfloat> hammer_mesh;
    std::vector<GLfloat> hammer_texture_coords;
    std::vector<GLfloat> hammer_normals;
    assimp_loaded_obj_to_internal(hammer_loaded_obj, &hammer_mesh, &hammer_texture_coords, &hammer_normals);

    const aiScene *hammer_stand_loaded_obj = importer.ReadFile("res/obj/hammer_stand.obj", aiProcess_Triangulate);

    std::vector<GLfloat> hammer_stand_mesh;
    std::vector<GLfloat> hammer_stand_texture_coords;
    std::vector<GLfloat> hammer_stand_normals;
    assimp_loaded_obj_to_internal(hammer_stand_loaded_obj, &hammer_stand_mesh,
                                  &hammer_stand_texture_coords, &hammer_stand_normals);

    srand(time(0));

    Object *objects[] = {
            (new SwingingObject(hammer_mesh.data(), hammer_mesh.size() * sizeof(hammer_mesh[0]), vec3(0, 5, 0),
                                hammer_normals.data(), hammer_normals.size() * sizeof(hammer_normals[0]),
                                SILVER_MATERIAL, 0.001f, 0.2))
                    ->scale(0.8),
            (new Object(hammer_stand_mesh.data(), hammer_stand_mesh.size() * sizeof(hammer_stand_mesh[0]),
                        vec3(0, 4.9, 0), hammer_stand_normals.data(),
                        hammer_stand_normals.size() * sizeof(hammer_stand_normals[0]), RED_PLASTIC)),
            (new SpinningObject(cube_mesh, sizeof(cube_mesh), vec3(-4, 2, 0), cube_normals, sizeof(cube_normals),
                                RED_PLASTIC))
                    ->set_color(cube_different_color, sizeof(cube_different_color)),
            (new SpinningObject(cube_mesh, sizeof(cube_mesh), vec3(4, 2, 0), cube_normals, sizeof(cube_normals),
                                RED_PLASTIC))
                    ->set_color(cube_purple_color, sizeof(cube_purple_color)),
            (new Object(down_plane_mesh, sizeof(down_plane_mesh), vec3(0, -5, 0), down_plane_normals,
                        sizeof(down_plane_normals), GREEN_RUBER))
                    ->set_texture(grass_texture, down_plane_texture_coords, sizeof(down_plane_texture_coords)),
    };
    Scene scene(objects, sizeof(objects) / sizeof(*objects));

    // main loop
    while (!glfwWindowShouldClose(window)) {

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        program.StartUseShader();

        scene.draw(program, get_cur_time_millis());

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    scene.delete_it();
    glfwTerminate();
    glDeleteVertexArrays(1, &vao);

    return 0;
}
