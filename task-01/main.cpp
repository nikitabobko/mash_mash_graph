//internal includes
#include "common.h"
#include "ShaderProgram.h"
#include "LiteMath.h"
#include <thread>
#include <chrono>

//External dependencies
#define GLFW_DLL

#include <GLFW/glfw3.h>
#include <random>

static GLsizei WIDTH = 1024, HEIGHT = 512; //размеры окна

using namespace LiteMath;

int scene_id = 0;

float3 get_default_cam_pos() {
    if (scene_id == 0) {
        return float3(2424.241455, 260.193848, 1651.539795);
    } else if (scene_id == 1) {
        return float3(-225.214020, -32.510338, 223.160461);
    }
    throw "Oops";
}

float get_default_along_yz_rot() {
    if (scene_id == 0) {
        return 0.150000;
    } else if (scene_id == 1) {
        return -0.040000;
    }
    throw "Oops";
}

float get_default_along_xz_rot() {
    if (scene_id == 0) {
        return -0.930000;
    } else if (scene_id == 1) {
        return 0.760000;
    }
    throw "Oops";
}

float3 default_cam_dir = float3(0, 0, -1);
float3 default_cam_y = float3(0, 1, 0);
float3 default_cam_x = float3(1, 0, 0);

float3 cam_pos = get_default_cam_pos();

double along_yz_rot = get_default_along_yz_rot();
double along_xz_rot = get_default_along_xz_rot();

const float DEFAULT_MOVE_SPEED = 40.0f;

const float MULT_SPEED = 3.f;

float cur_cam_dir_speed = 0;
float cur_cam_x_speed = 0;
float cur_cam_y_speed = 0;

void windowResize(GLFWwindow *window, int width, int height) {
    WIDTH = width;
    HEIGHT = height;
}

bool focused = true;

bool first = true;

void reset_pos() {
    first = true;
    cam_pos = get_default_cam_pos();
    along_xz_rot = get_default_along_xz_rot();
    along_yz_rot = get_default_along_yz_rot();
}

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
            reset_pos();
        } else if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT) {
            cur_cam_dir_speed *= MULT_SPEED;
            cur_cam_x_speed *= MULT_SPEED;
            cur_cam_y_speed *= MULT_SPEED;
        } else if (key == GLFW_KEY_1) {
            scene_id = 0;
            reset_pos();
        } else if (key == GLFW_KEY_2) {
            scene_id = 1;
            reset_pos();
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
            cur_cam_dir_speed /= MULT_SPEED;
            cur_cam_x_speed /= MULT_SPEED;
            cur_cam_y_speed /= MULT_SPEED;
        }
    }
}

double prev_x = 0;
double prev_y = 0;

static inline double minOfDouble(double a, double b) {
    return a < b ? a : b;
}

static inline double maxOfDouble(double a, double b) {
    return a > b ? a : b;
}

static void mouseMove(GLFWwindow *window, double x, double y) {
    if (!focused) {
        return;
    }

    if (first) {
        prev_x = x;
        prev_y = y;
        first = false;
        return;
    }

    double x_diff = x - prev_x;
    double y_diff = y - prev_y;
    prev_x = x;
    prev_y = y;

    along_xz_rot += x_diff / 100;
    along_yz_rot += y_diff / 100;

    along_yz_rot = maxOfDouble(minOfDouble(along_yz_rot, M_PI_2), -M_PI_2);
}

static void calculate_cur_cam_pos(float3 cam_dir, float3 cam_x, float3 cam_y) {
    cam_pos += cam_dir.normalized() * cur_cam_dir_speed;
    cam_pos += cam_x * cur_cam_x_speed;
    cam_pos += cam_y * cur_cam_y_speed;
}


int initGL() {
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
    if (!focused) {
        first = true;
    }
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

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "", nullptr, nullptr);
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
        glVertexAttribPointer(vertexLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
        GL_CHECK_ERRORS;

        glBindVertexArray(0);
    }

    //цикл обработки сообщений и отрисовки сцены каждый кадр
    while (!glfwWindowShouldClose(window)) {
        char title_buf[2048];
        snprintf(title_buf, sizeof(title_buf), "OpenGL ray marching sample. Scene_id: %d", scene_id);

        glfwPollEvents();

        if (!focused) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            continue;
        }

        //очищаем экран каждый кадр
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        GL_CHECK_ERRORS;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        GL_CHECK_ERRORS;

        program.StartUseShader();
        GL_CHECK_ERRORS;

        const float along_xz_matrix[] = {
                (float) cos(along_xz_rot), 0, (float) -sin(along_xz_rot),
                0, 1, 0,
                (float) sin(along_xz_rot), 0, (float) cos(along_xz_rot)
        };
        float3x3 rotate_along_xz = float3x3(along_xz_matrix);

        const float along_yz_matrix[] = {
                1, 0, 0,
                0, (float) cos(along_yz_rot), (float) sin(along_yz_rot),
                0, (float) -sin(along_yz_rot), (float) cos(along_yz_rot)
        };
        float3x3 rotate_along_yz = float3x3(along_yz_matrix);

        float3x3 rotate = mul(rotate_along_xz, rotate_along_yz);

        float3 cam_y = rotate*default_cam_y;
        float3 cam_x = rotate*default_cam_x;

        float3 cam_dir = rotate*default_cam_dir;

//        printf("along_xz_rot: %lf\n", along_xz_rot);
//        printf("along_yz_rot: %lf\n", along_yz_rot);

        calculate_cur_cam_pos(cam_dir, cam_x, cam_y);

//        printf("cam_pos: %lf %lf %lf\n", cam_pos.x, cam_pos.y, cam_pos.z);

        glfwSetWindowTitle(window, title_buf);

        program.SetUniform("g_screenWidth", WIDTH);
        program.SetUniform("g_screenHeight", HEIGHT);
        program.SetUniform("cam_pos", cam_pos);
        program.SetUniform("cam_dir", cam_dir);
        program.SetUniform("cam_x", cam_x);
        program.SetUniform("cam_y", cam_y);
        program.SetUniform("scene_id", scene_id);

        // очистка и заполнение экрана цветом
        glViewport(0, 0, WIDTH, HEIGHT);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // draw call
        glBindVertexArray(g_vertexArrayObject);
        GL_CHECK_ERRORS;
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        GL_CHECK_ERRORS;  // The last parameter of glDrawArrays is equal to VS invocations

        program.StopUseShader();

        glfwSwapBuffers(window);
    }

    //очищаем vboи vao перед закрытием программы
    glDeleteVertexArrays(1, &g_vertexArrayObject);
    glDeleteBuffers(1, &g_vertexBufferObject);

    glfwTerminate();
    return 0;
}
