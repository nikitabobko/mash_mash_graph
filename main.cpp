//internal includes
#include "common.h"
#include "ShaderProgram.h"
#include "LiteMath.h"
#include <thread>
#include <chrono>

//External dependencies
#define GLFW_DLL

typedef const float pDouble[9];

#include <GLFW/glfw3.h>
#include <random>

static GLsizei WIDTH = 1920, HEIGHT = 1080; //размеры окна

using namespace LiteMath;

float cam_rot[2] = {0, 0};

float3 default_cam_pos = float3(0, 0, 1500);
float3 default_cam_dir = float3(0, 0, -1);
float3 default_cam_y = float3(0, 1, 0);
float3 default_cam_x = float3(1, 0, 0);

float3 cam_pos = default_cam_pos;

float3 cam_dir = default_cam_dir;

float3 cam_y = default_cam_y;

float3 cam_x = default_cam_x;

const float DEFAULT_MOVE_SPEED = 50.0f;

float cur_cam_dir_speed = 0;
float cur_cam_x_speed = 0;
float cur_cam_y_speed = 0;

void windowResize(GLFWwindow *window, int width, int height) {
    WIDTH = width;
    HEIGHT = height;
}

bool focused = true;

bool first = true;

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (!focused) {
        return;
    }
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, 1);
        return;
    }
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_W) {
            cur_cam_dir_speed = DEFAULT_MOVE_SPEED;
        } else if (key == GLFW_KEY_S) {
            cur_cam_dir_speed = -DEFAULT_MOVE_SPEED;
        } else if (key == GLFW_KEY_D) {
            cur_cam_x_speed = DEFAULT_MOVE_SPEED;
        } else if (key == GLFW_KEY_A) {
            cur_cam_x_speed = -DEFAULT_MOVE_SPEED;
        } else if (key == GLFW_KEY_F) {
            cur_cam_y_speed = -DEFAULT_MOVE_SPEED;
        } else if (key == GLFW_KEY_R) {
            cur_cam_y_speed = DEFAULT_MOVE_SPEED;
        } else if (key == GLFW_KEY_0) {
            first = true;
            cam_pos = default_cam_pos;
            cam_dir = default_cam_dir;
            cam_x = default_cam_x;
            cam_y = default_cam_y;
        } else if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT) {
            cur_cam_dir_speed *= 2;
            cur_cam_x_speed *= 2;
            cur_cam_y_speed *= 2;
        }
    } else if (action == GLFW_RELEASE) {
        if (key == GLFW_KEY_W) {
            cur_cam_dir_speed = 0;
        } else if (key == GLFW_KEY_S) {
            cur_cam_dir_speed = 0;
        } else if (key == GLFW_KEY_D) {
            cur_cam_x_speed = 0;
        } else if (key == GLFW_KEY_A) {
            cur_cam_x_speed = 0;
        } else if (key == GLFW_KEY_F) {
            cur_cam_y_speed = 0;
        } else if (key == GLFW_KEY_R) {
            cur_cam_y_speed = 0;
        } else if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT) {
            cur_cam_dir_speed /= 2;
            cur_cam_x_speed /= 2;
            cur_cam_y_speed /= 2;
        }
    }
}

int initial_xpos = 0;
int initial_ypos = 0;

static float3 vector_in_basis(const float3 &components, const float3 &x, const float3 &y, const float3 &z) {
    return components.x * x + components.y * y + components.z * z;
}

static void mouseMove(GLFWwindow *window, double x, double y) {
    if (!focused) {
        return;
    }
    int xpos = (int)x % WIDTH;
    int ypos = (int)y % HEIGHT;

    xpos -= WIDTH/2;
    ypos -= HEIGHT/2;

    if (first) {
        initial_xpos = xpos;
        initial_ypos = ypos;
        first = false;
        return;
    }

    xpos -= initial_xpos;
    ypos -= initial_ypos;

    double along_xz = 1. * xpos / (WIDTH/2) * M_PI;
    double along_yz = 1. * ypos / (HEIGHT/2) * M_PI;

    const float along_xz_matrix[] = {
            (float) cos(along_xz), 0, (float) -sin(along_xz),
            0, 1, 0,
            (float) sin(along_xz), 0, (float) cos(along_xz)
    };
    float3x3 rotate_along_xz = float3x3(along_xz_matrix);

    const float along_yz_matrix[] = {
            1, 0, 0,
            0, (float) cos(along_yz), (float) sin(along_yz),
            0, (float) -sin(along_yz), (float) cos(along_yz)
    };
    float3x3 rotate_along_yz = float3x3(along_yz_matrix);

    float3x3 rotate = mul(rotate_along_xz, rotate_along_yz);

    cam_y = rotate*default_cam_y;
    cam_x = rotate*default_cam_x;

    cam_dir = rotate*default_cam_dir;

    printf("%lf==0 %lf==0 %lf==0\n", dot(cam_dir, cam_y), dot(cam_dir, cam_x), dot(cam_x, cam_y));
}

static void calculate_cur_cam_pos() {
    cam_pos += cam_dir.normalized() * cur_cam_dir_speed;
    cam_pos += cam_x * cur_cam_x_speed;
    cam_pos += cam_y * cur_cam_y_speed;
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

static void focuse_changed_callback(GLFWwindow *window, int focused_local) {
    focused = focused_local != 0;
}

int main(int argc, char **argv) {
    if (!glfwInit()) {
        return -1;
    }

    //запрашиваем контекст opengl версии 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL ray marching sample",  glfwGetPrimaryMonitor(), nullptr);
    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouseMove);
    glfwSetKeyCallback(window, key_callback);
    glfwSetWindowSizeCallback(window, windowResize);
    glfwMakeContextCurrent(window);
    glfwSetWindowFocusCallback(window, focuse_changed_callback);

    if (initGL() != 0)
        return -1;

    //Reset any OpenGL errors which could be present for some reason
    GLenum gl_error = glGetError();
    while (gl_error != GL_NO_ERROR)
        gl_error = glGetError();

    //создание шейдерной программы из двух файлов с исходниками шейдеров
    //используется класс-обертка ShaderProgram
    std::unordered_map<GLenum, std::string> shaders;
    shaders[GL_VERTEX_SHADER] = "vertex.glsl";
    shaders[GL_FRAGMENT_SHADER] = "fragment.glsl";
    ShaderProgram program(shaders);
    GL_CHECK_ERRORS;

    glfwSwapInterval(1); // force 60 frames per second

    //Создаем и загружаем геометрию поверхности
    //
    GLuint g_vertexBufferObject;
    GLuint g_vertexArrayObject;
    {

        float quadPos[] = {
                -1.0f, 1.0f,     // v0 - top left corner
                -1.0f, -1.0f,    // v1 - bottom left corner
                1.0f, 1.0f,      // v2 - top right corner
                1.0f, -1.0f      // v3 - bottom right corner
        };

        g_vertexBufferObject = 0;
        GLuint vertexLocation = 0; // simple layout, assume have only positions at location = 0

        glGenBuffers(1, &g_vertexBufferObject);
        GL_CHECK_ERRORS;
        glBindBuffer(GL_ARRAY_BUFFER, g_vertexBufferObject);
        GL_CHECK_ERRORS;
        glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(GLfloat), (GLfloat *) quadPos, GL_STATIC_DRAW);
        GL_CHECK_ERRORS;

        glGenVertexArrays(1, &g_vertexArrayObject);
        GL_CHECK_ERRORS;
        glBindVertexArray(g_vertexArrayObject);
        GL_CHECK_ERRORS;

        glBindBuffer(GL_ARRAY_BUFFER, g_vertexBufferObject);
        GL_CHECK_ERRORS;
        glEnableVertexAttribArray(vertexLocation);
        GL_CHECK_ERRORS;
        glVertexAttribPointer(vertexLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
        GL_CHECK_ERRORS;

        glBindVertexArray(0);
    }

    //цикл обработки сообщений и отрисовки сцены каждый кадр
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (!focused) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            first = true;
            continue;
        }

        //очищаем экран каждый кадр
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        GL_CHECK_ERRORS;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        GL_CHECK_ERRORS;

        program.StartUseShader();
        GL_CHECK_ERRORS;

        float4x4 camRotMatrix = mul(rotate_Y_4x4(-cam_rot[1]), rotate_X_4x4(+cam_rot[0]));
        float4x4 camTransMatrix = translate4x4(cam_pos);
        float4x4 rayMatrix = mul(camRotMatrix, camTransMatrix);
//        program.SetUniform("g_rayMatrix", rayMatrix);

        calculate_cur_cam_pos();

        program.SetUniform("g_screenWidth", WIDTH);
        program.SetUniform("g_screenHeight", HEIGHT);
        program.SetUniform("cam_pos", cam_pos);
        program.SetUniform("cam_dir", cam_dir);
        program.SetUniform("cam_x", cam_x);
        program.SetUniform("cam_y", cam_y);

        // очистка и заполнение экрана цветом
        //
        glViewport(0, 0, WIDTH, HEIGHT);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // draw call
        //
        glBindVertexArray(g_vertexArrayObject);
        GL_CHECK_ERRORS;
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        GL_CHECK_ERRORS;  // The last parameter of glDrawArrays is equal to VS invocations

        program.StopUseShader();

        glfwSwapBuffers(window);
    }

    //очищаем vboи vao перед закрытием программы
    //
    glDeleteVertexArrays(1, &g_vertexArrayObject);
    glDeleteBuffers(1, &g_vertexBufferObject);

    glfwTerminate();
    return 0;
}
