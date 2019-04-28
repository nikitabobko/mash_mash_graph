#include "common.h"
#include "core.cpp"
#include "ShaderProgram.h"
#include "util.h"

#define GLFW_DLL

#include <GLFW/glfw3.h>
#include <random>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

static const GLsizei WIDTH = 640, HEIGHT = 480; //размеры окна

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

    GLfloat triangle_mesh[] = {
            -1.0f, -1.0f, 0.0f,
            1.0f, -1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
    };
    Object *objects[] = {
            new SpinningObject(triangle_mesh, sizeof(triangle_mesh), 0.001f, glm::vec3(2, 0, 0.5)),
            new SpinningObject(triangle_mesh, sizeof(triangle_mesh), 0.002f, glm::vec3(-1, 0, -1)),
    };
    Scene scene(objects, sizeof(objects) / sizeof(*objects));

    // main loop
    while (!glfwWindowShouldClose(window)) {

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT);

        program.StartUseShader();

        scene.draw(program, get_cur_time_millis());

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Close OpenGL window and terminate GLFW
    glfwTerminate();
    glDeleteVertexArrays(1, &vao);

    return 0;
}
