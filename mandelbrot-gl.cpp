// OpenGL mandelbrot explorer
// Credits :
// "Implementation of float-float operators on graphics hardware" (Da Graçca & Defour 2006)
// "Extended-Precision Floating-Point Numbers for GPU Computation" (Andrew Thall, 2007)
// "Heavy computing with GLSL"  H.Thasler https://www.thasler.com/blog/blog/glsl-part2-emu
// "Emulated 64-bit floats in OpenGL ES shader"  https://betelge.wordpress.com/2016/08/14/emulated-64-bit-floats-in-opengl-es-shader/
// Original OpenGL boilerplate code from learnopengl.com tutorial

#define NOMINMAX

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <cstdint>
#include <cinttypes>
#include <chrono>
#include <algorithm>

#include <fstream>

#include <CoreMacros.hpp>
#include <CoreStrings.hpp>

#include "Shader.hpp"
#include "FloatFloat.hpp"

// glfw callbacks
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void keyboard_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

// initial settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;

// Shaders available
enum ShaderType
{
    SHADER_FLOAT = 0,  // Basic shader with floating-point precision
    SHADER_FLOATFLOAT, // Double precision emulation with 2 floats
    SHADER_DOUBLE,     // True double precision
    MAX_SHADERS        // Total number of available shaders
};

// Container for the uniforms of a shader
struct Uniforms
{
    GLint centerUniform;   // center coordinates
    GLint scaleUniform;    // zoom level
    GLint ratioUniform;    // aspect ratio
    GLint maxItersUniform; // max number of mandelbrot function iterations
};

// Global variable containing the parameters of current view
struct Context
{
    // View parameters
    double centerX{-0.5}; // center point x
    double centerY{0.0};  // center point y
    double scale{2};      // zoom level
    double ratio{1.0};    // aspect ratio
    uint iters{1000};     // max number of Mandelbrot function iterations

    // Shaders and related data
    ShaderType current_shader{SHADER_FLOAT};
    std::unique_ptr<ShaderProgram> shaders[MAX_SHADERS];
    Uniforms uniforms[MAX_SHADERS];

} g_context;

void save(const Context &context, const std::string &filename)
{
    std::string saveString;

    // Print the floats to hex to get the exact value
    saveString += core::double2hex(context.centerX);
    saveString += " ";
    saveString += core::double2hex(context.centerY);
    saveString += " ";
    saveString += core::double2hex(context.scale);

    std::ofstream of{filename};
    CORE_ASSERT(of.good(), "Can't open " << filename);
    of << saveString;
}

void load(Context &context, const std::string &filename)
{
    static_assert(sizeof(context.centerX) == sizeof(uint64), "Assuming 64 bits double");

    union {
        double f;
        uint64 i;
    } x, y, s;

    std::ifstream in(filename);

    CORE_ASSERT(in.good(), " Can't open " << filename);

    std::string l;
    std::getline(in, l);
    auto vec = core::splitString(l, ' ');

    CORE_ASSERT(vec.size() == 3, "Incorrect file format");

    x.i = _strtoui64(vec[0].c_str(), NULL, 16);
    y.i = _strtoui64(vec[1].c_str(), NULL, 16);
    s.i = _strtoui64(vec[2].c_str(), NULL, 16);

    context.centerX = x.f;
    context.centerY = y.f;
    context.scale = s.f;
}

// Update the shader uniforms
void updateUniforms(const Context &context)
{
    CORE_ASSERT(context.current_shader < MAX_SHADERS, "Invalid shader");
    const Uniforms &u = context.uniforms[context.current_shader];
    switch (context.current_shader)
    {
    case SHADER_FLOAT:
    {
        GL_ASSERT(glUniform2f(u.centerUniform, static_cast<GLfloat>(context.centerX),
                              static_cast<GLfloat>(context.centerY)));
        GL_ASSERT(glUniform1f(u.scaleUniform, static_cast<GLfloat>(context.scale)));
        GL_ASSERT(glUniform1f(u.ratioUniform, static_cast<GLfloat>(context.ratio)));
        GL_ASSERT(glUniform1ui(u.maxItersUniform, static_cast<GLuint>(context.iters)));
        break;
    }
    case SHADER_FLOATFLOAT:
    {
        FloatFloat center[2] = {context.centerX, context.centerY};
        FloatFloat s(context.scale);
        static_assert(sizeof(center) == 4 * sizeof(float), "Size/align problem ");

        GL_ASSERT(glUniform4fv(u.centerUniform, 1, center[0].values));
        GL_ASSERT(glUniform2fv(u.scaleUniform, 1, s.values));
        GL_ASSERT(glUniform1f(u.ratioUniform, static_cast<GLfloat>(context.ratio)));
        GL_ASSERT(glUniform1ui(u.maxItersUniform, static_cast<GLuint>(context.iters)));
        break;
    }
    case SHADER_DOUBLE:
    {
        GL_ASSERT(glUniform2d(u.centerUniform, static_cast<GLdouble>(context.centerX),
                              static_cast<GLdouble>(context.centerY)));
        GL_ASSERT(glUniform1d(u.scaleUniform, static_cast<GLdouble>(context.scale)));
        GL_ASSERT(glUniform1f(u.ratioUniform, static_cast<GLfloat>(context.ratio)));
        GL_ASSERT(glUniform1ui(u.maxItersUniform, static_cast<GLuint>(context.iters)));
        break;
    }
    default:
        CORE_ASSERT(false, "should not get here");
    }
}

using ns_clock = std::chrono::high_resolution_clock;

class FPSMonitor
{
  public:
    FPSMonitor(uint avg = 100)
    {
        setAvgFrames(avg);
    }
    void report(const ns_clock::time_point &start,
                const ns_clock::time_point &update,
                const ns_clock::time_point &render,
                const ns_clock::time_point &swap,
                const ns_clock::time_point &end)
    {
        updateTime += ns_clock::duration(update - start).count();
        renderTime += ns_clock::duration(render - update).count();
        swapTime += ns_clock::duration(swap - render).count();
        uiTime += ns_clock::duration(end - swap).count();
        if (++frameCounter == avgFrames)
        {
            print();
            reset();
        }
    }

    void print()
    {
        auto f = [avgFrames = avgFrames](const std::string &name, uint64 t) {
            CORE_ASSERT(name.length() < 10, "");
            std::string spaces = std::string(10 - name.length() + 1, ' ');

            std::cout << name << spaces << t / avgFrames << " ns ("
                      << double(avgFrames * 1e9) / double(t) << " fps )"
                      << std::endl;
        };
        f("Update", updateTime);
        f("Render", renderTime);
        f("Swap", swapTime);
        f("UI", uiTime);
        std::cout << std::endl;
    }

    void setAvgFrames(uint avg)
    {
        // ensure we average over at least one frame
        avgFrames = std::max(1u, avg);
        reset();
    }

    void reset()
    {
        // reset data
        frameCounter = 0;
        updateTime = 0;
        renderTime = 0;
        swapTime = 0;
        uiTime = 0;
    }

  private:
    uint avgFrames;
    uint frameCounter;
    uint64 updateTime;
    uint64 renderTime;
    uint64 swapTime;
    uint64 uiTime;
};

FPSMonitor g_monitor(100);

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "MandelbrotGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, keyboard_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // We don't need to do depth tests, as we are in 2D.
    GL_ASSERT(glDisable(GL_DEPTH_TEST));

    // Print some Open GL boilerplate
    std::cout << "Renderer : " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "Vendor   : " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "OpenGL   : " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL     : " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    // build and compile our shader programs
    // -------------------------------------
    g_context.shaders[SHADER_FLOAT].reset(new ShaderProgram());
    g_context.shaders[SHADER_FLOAT]->loadShaderFiles("Vertex.glsl", "PixelF.glsl");

    g_context.shaders[SHADER_FLOATFLOAT].reset(new ShaderProgram());
    g_context.shaders[SHADER_FLOATFLOAT]->loadShaderFiles("Vertex.glsl", "PixelFF.glsl");

    g_context.shaders[SHADER_DOUBLE].reset(new ShaderProgram());
    g_context.shaders[SHADER_DOUBLE]->loadShaderFiles("Vertex.glsl", "PixelD.glsl");

    // Initialize each shaders' uniform handles
    for (uint i = 0; i < MAX_SHADERS; ++i)
    {
        Uniforms &u = g_context.uniforms[i];
        const GLint id = g_context.shaders[i]->getID();
        GL_ASSERT(u.centerUniform = glGetUniformLocation(id, "center"));
        GL_ASSERT(u.scaleUniform = glGetUniformLocation(id, "scale"));
        GL_ASSERT(u.ratioUniform = glGetUniformLocation(id, "ratio"));
        GL_ASSERT(u.maxItersUniform = glGetUniformLocation(id, "max"));
    }

    // Set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    const float vertices[] = {
        1.0f, 1.0f, 0.0f,   // top right
        1.0f, -1.0f, 0.0f,  // bottom right
        -1.0f, -1.0f, 0.0f, // bottom left
        -1.0f, 1.0f, 0.0f,  // top left
    };
    const uint indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    GLuint VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    ns_clock::time_point start, update, render, swap, end;

    while (!glfwWindowShouldClose(window))
    {
        start = std::chrono::high_resolution_clock::now();

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // select current shader
        g_context.shaders[g_context.current_shader]->useProgram();
        updateUniforms(g_context);

        update = std::chrono::high_resolution_clock::now();

        // draw
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        render = std::chrono::high_resolution_clock::now();

        // glfw: swap buffers
        glfwSwapBuffers(window);

        swap = std::chrono::high_resolution_clock::now();

        // glfw : ui
        glfwPollEvents();

        end = std::chrono::high_resolution_clock::now();

        g_monitor.report(start, update, render, swap, end);
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();

    return 0;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
    g_context.ratio = float(width) / float(height);
}

// Keyboard controls
// ESC : quit
// Arrow keys : move view
// Z / X : zoom + / zoom -
// I/ J : increase / decrease iterations
// P : print current view coordinates
// S : cycle shaders
// F5 : save current view coordinates
// F9 : load saved coordinates
void keyboard_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    const float sensitivity = 100.f; // Input sensitivity.
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        switch (key)
        {
        case GLFW_KEY_ESCAPE:
        {
            glfwSetWindowShouldClose(window, true);
            break;
        }
        case GLFW_KEY_UP:
        {
            g_context.centerY += (g_context.scale / sensitivity);
            break;
        }
        case GLFW_KEY_DOWN:
        {
            g_context.centerY -= (g_context.scale / sensitivity);
            break;
        }
        case GLFW_KEY_RIGHT:
        {
            g_context.centerX += (g_context.scale / sensitivity);
            break;
        }
        case GLFW_KEY_LEFT:
        {
            g_context.centerX -= (g_context.scale / sensitivity);
            break;
        }
        case GLFW_KEY_Z:
        {
            g_context.scale *= (1.0f - 1 / sensitivity);
            break;
        }
        case GLFW_KEY_X:
        {
            g_context.scale *= (1.0f + 1 / sensitivity);
            break;
        }
        case GLFW_KEY_I:
        {
            g_context.iters = std::min(100000u, g_context.iters * 10);
            break;
        }
        case GLFW_KEY_J:
        {
            g_context.iters = std::max(1u, g_context.iters / 10);
            break;
        }
        case GLFW_KEY_P:
        {
            if (action == GLFW_PRESS)
            {
                std::cout << g_context.centerX << " "
                          << g_context.centerY << " "
                          << g_context.scale
                          << "(" << g_context.iters << ")" << std::endl;
            }
            break;
        }
        case GLFW_KEY_S:
        {
            if (action == GLFW_PRESS)
            {
                g_context.current_shader = ShaderType((g_context.current_shader + 1) % MAX_SHADERS);
                std::cout << " switching to shader" << g_context.current_shader << std::endl;
                g_monitor.reset();
            }
            break;
        }
        case GLFW_KEY_F5:
        {
            save(g_context, "mbrot.sav");
            break;
        }
        case GLFW_KEY_F9:
        {
            load(g_context, "mbrot.sav");
            break;
        }
        } // switch
    }     // if (pressed)
}
