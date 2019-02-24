//internal includes
#include "common.h"
#include "ShaderProgram.h"
#include "LiteMath.h"

//External dependencies
#define GLFW_DLL

typedef const float pDouble[9];

#include <GLFW/glfw3.h>
#include <random>

static GLsizei WIDTH = 1920, HEIGHT = 1080; //размеры окна

using namespace LiteMath;

float cam_rot[2] = {0, 0};

float3 default_cam_pos = float3(0, 0, 1500);
float3 default_cam_dir = float3(0, 0, -1000);
float3 default_cam_y = float3(0, 1, 0);
float3 default_cam_x = float3(1, 0, 0);

float3 cam_pos = default_cam_pos;

float3 cam_dir = default_cam_dir;

float3 cam_y = default_cam_y;

float3 cam_x = default_cam_x;

void windowResize(GLFWwindow *window, int width, int height) {
    WIDTH = width;
    HEIGHT = height;
}

bool first = true;

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
//    if (action == GLFW_REPEAT) {
        if (key == GLFW_KEY_W) {
            cam_pos += 50*cam_dir.normalized();
//            cam_pos.z -= 50;
        } else if (key == GLFW_KEY_S) {
            cam_pos -= 50*cam_dir.normalized();
//            cam_pos.z += 50;
        } else if (key == GLFW_KEY_D) {
            cam_pos += 50*cam_x;
//            cam_pos.x += 50;
        } else if (key == GLFW_KEY_A) {
            cam_pos -= 50*cam_x;
//            cam_pos.x -= 50;
        } else if (key == GLFW_KEY_F) {
            cam_pos -= 50*cam_y;
        } else if (key == GLFW_KEY_R) {
            cam_pos += 50*cam_y;
        }
//        printf("x: %lf, y: %lf, z: %lf\n", cam_pos.x, cam_pos.y, cam_pos.z);
//    }
//    GLFW_RELEASE;
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_0) {
            first = true;
            cam_pos = default_cam_pos;
            cam_dir = default_cam_dir;
            cam_x = default_cam_x;
            cam_y = default_cam_y;
        }
    }
//        activate_airship();
}

int initial_xpos = 0;
int initial_ypos = 0;

static float3 vector_in_basis(const float3 &components, const float3 &x, const float3 &y, const float3 &z) {
    return components.x * x + components.y * y + components.z * z;
}

static void mouseMove(GLFWwindow *window, double x, double y) {
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


//    xpos -= prev_x;
//    ypos -= prev_y;

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

    cam_y = rotate_along_yz*default_cam_y;
    cam_x = rotate_along_xz*default_cam_x;

    cam_dir = length(default_cam_dir)*(rotate_along_xz*(rotate_along_yz*default_cam_dir.normalized()));

    printf("%lf==0 %lf==0 %lf==0\n", dot(cam_dir, cam_y), dot(cam_dir, cam_x), dot(cam_x, cam_y));
//    cam_dir = length(default_cam_dir)*(*default_cam_dir.normalized());


//    float3 components = ;

//
//    cam_dir = cam_pos + rotate_along_yz*(rotate_along_xz*(cam_dir - cam_pos));
//    cam_x = cam_pos + rotate_along_yz*(rotate_along_xz*(cam_x - cam_pos));
//    cam_y = cam_pos + rotate_along_yz*(rotate_along_xz*(cam_y - cam_pos));

//    printf("xpos: %lf/%d\n", xpos, WIDTH);
//    printf("ypos: %lf/%d\n", ypos, HEIGHT);

//    xpos *= 0.05f;
//    ypos *= 0.05f;
//
//    int x1 = int(xpos);
//    int y1 = int(ypos);
//
//    cam_rot[0] -= 0.25f * (y1 - my);    //Изменение угола поворота
//    cam_rot[1] -= 0.25f * (x1 - mx);
//
//    mx = int(xpos);
//    my = int(ypos);
//    prev_x = xpos;
//    prev_y = ypos;
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
