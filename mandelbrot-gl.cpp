// OpenGL mandelbrot explorer
// Credits :
// "Implementation of float-float operators on graphics hardware" (Da Graçca & Defour 2006)
// "Extended-Precision Floating-Point Numbers for GPU Computation" (Andrew Thall, 2007)
// "Heavy computing with GLSL"  H.Thasler https://www.thasler.com/blog/blog/glsl-part2-emu
// "Emulated 64-bit floats in OpenGL ES shader"  https://betelge.wordpress.com/2016/08/14/emulated-64-bit-floats-in-opengl-es-shader/
#define NOMINMAX

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <cstdint>
#include <cinttypes>
#include <algorithm>

#include <CoreMacros.hpp>

#include "Shader.hpp"
#include "FloatFloat.hpp"


// glfw callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

// initial settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;

// Shaders available
enum ShaderType
{
    SHADER_FLOAT = 0, // Basic shader with floating-point precision
    SHADER_FLOATFLOAT, // Double precision emulation with 2 floats
    MAX_SHADERS // Total number of available shaders
};

// Container for the uniforms of a shader
struct Uniforms
{
    GLint centerUniform; // center coordinates
    GLint scaleUniform;  // zoom level
    GLint ratioUniform;  // aspect ratio
    GLint maxItersUniform;  // max number of mandelbrot function iterations
};

// Global variable containing the parameters of current view
struct Context
{
    // View parameters
    double centerX{-0.5}; // center point x
    double centerY{0.0};  // center point y
    double scale{2};      // zoom level
    double ratio{1.0};    // aspect ratio
    uint   iters{1000};   // max number of Mandelbrot function iterations

    // Shaders and related data
    ShaderType current_shader {SHADER_FLOAT};
    std::unique_ptr<ShaderProgram> shaders[MAX_SHADERS];
    Uniforms uniforms[MAX_SHADERS];

} g_context;

// Update the shader uniforms
void updateUniforms( const Context& context )
{
    CORE_ASSERT( context.current_shader < MAX_SHADERS, "Invalid shader");
    const Uniforms& u = context.uniforms[context.current_shader];
    switch (context.current_shader)
    {
        case SHADER_FLOAT:
        {
            GL_ASSERT(glUniform2f(u.centerUniform, static_cast<GLfloat>(context.centerX),
                                                   static_cast<GLfloat>(context.centerY)));
            GL_ASSERT(glUniform1f(u.scaleUniform, static_cast<GLfloat>(context.scale)));
            GL_ASSERT(glUniform1f(u.ratioUniform, static_cast<GLfloat>(context.ratio)));
            break;
        }
        case SHADER_FLOATFLOAT:
        {
            FloatFloat center[2] = { context.centerX, context.centerY };
            FloatFloat s(context.scale);
            static_assert( sizeof(center) == 4 * sizeof(float), "Size/align problem " );

            GL_ASSERT(glUniform4fv(u.centerUniform, 1, center[0].values));
            GL_ASSERT(glUniform2fv(u.scaleUniform, 1, s.values));
            GL_ASSERT(glUniform1f(u.ratioUniform, static_cast<GLfloat>(context.ratio)));
            break;
        }
        default:
            CORE_ASSERT(false, "should not get here");
    }
}

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
    GL_ASSERT(glDisable( GL_DEPTH_TEST ) );

    // Print some Open GL boilerplate
    std::cout << "Renderer : " << glGetString( GL_RENDERER ) << std::endl;
    std::cout << "Vendor   : " << glGetString( GL_VENDOR ) << std::endl;
    std::cout << "OpenGL   : " << glGetString( GL_VERSION ) << std::endl;
    std::cout << "GLSL     : " << glGetString( GL_SHADING_LANGUAGE_VERSION ) <<std::endl;

    // build and compile our shader programs
    // -------------------------------------
    g_context.shaders[SHADER_FLOAT].reset(new ShaderProgram());
    g_context.shaders[SHADER_FLOAT]->loadShaderFiles("Vertex.glsl","PixelF.glsl");

    g_context.shaders[SHADER_FLOATFLOAT].reset(new ShaderProgram());
    g_context.shaders[SHADER_FLOATFLOAT]->loadShaderFiles("Vertex.glsl","PixelFF.glsl");

    // Initialize each shaders' uniform handles
    for (uint i = 0; i < MAX_SHADERS; ++i)
    {
        Uniforms& u = g_context.uniforms[i];
        const GLint id = g_context.shaders[i]->getID();
        GL_ASSERT(u.centerUniform   = glGetUniformLocation(id, "center"));
        GL_ASSERT(u.scaleUniform    = glGetUniformLocation(id, "scale"));
        GL_ASSERT(u.ratioUniform    = glGetUniformLocation(id, "ratio"));
        GL_ASSERT(u.maxItersUniform = glGetUniformLocation(id, "max"));
    }


    // Set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    const float vertices[] = {
         1.0f,  1.0f, 0.0f,   // top right
         1.0f, -1.0f, 0.0f,  // bottom right
        -1.0f, -1.0f, 0.0f, // bottom left
        -1.0f,  1.0f, 0.0f,   // top left
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

    while (!glfwWindowShouldClose(window))
    {
        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // select current shader
        g_context.shaders[g_context.current_shader]->useProgram();
        updateUniforms(g_context);

        // draw
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
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
                g_context.scale *= (1.0f - 1/sensitivity);
                break;
            }
            case GLFW_KEY_X:
            {
                g_context.scale *= (1.0f + 1/sensitivity);
                break;
            }
            case GLFW_KEY_P:
            {
                if (action == GLFW_PRESS)
                {
                    std::cout << g_context.centerX << " " << g_context.centerY << " " << g_context.scale << std::endl;
                }
                break;
            }
            case GLFW_KEY_S:
            {
                if (action == GLFW_PRESS)
                {
                    g_context.current_shader = ShaderType((g_context.current_shader+ 1) % MAX_SHADERS);
                    std::cout<<" switching to shader" << g_context.current_shader<<std::endl;
                }
                break;
            }
        } // switch
    } // if (pressed)
}
