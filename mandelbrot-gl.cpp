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

// shader interface for the precision shaders
class MandelbrotShader: public ShaderProgram
{
    public:
    virtual void update( double scale, double centerX, double centerY, double ratio) = 0;
};

/// Single precision shader
class SPMandelbrotShader : public MandelbrotShader
{
    public:
    SPMandelbrotShader( ) : MandelbrotShader()
    {
        loadShaderFiles("Vertex.glsl","PixelF.glsl");
        GL_ASSERT(m_centerUniform = glGetUniformLocation(getID(), "center"));
        GL_ASSERT(m_scaleUniform  = glGetUniformLocation(getID(), "scale"));
        GL_ASSERT(m_ratioUniform  = glGetUniformLocation(getID(), "ratio"));
    }

    virtual void update( double scale, double centerX, double centerY, double ratio) override
    {
        glUniform2f(m_centerUniform, static_cast<GLfloat>(centerX), static_cast<GLfloat>(centerY));
        glUniform1f(m_scaleUniform, static_cast<GLfloat>(scale));
        glUniform1f(m_ratioUniform, static_cast<GLfloat>(ratio));
    }

    private:
    GLint m_centerUniform;
    GLint m_scaleUniform;
    GLint m_ratioUniform;
};

/// Double-precision shader
class DPMandelbrotShader : public MandelbrotShader
{
    public:
    DPMandelbrotShader( ) : MandelbrotShader()
    {
        loadShaderFiles("Vertex.glsl","PixelFF.glsl");
        GL_ASSERT(m_centerUniform = glGetUniformLocation(getID(), "center"));
        GL_ASSERT(m_scaleUniform  = glGetUniformLocation(getID(), "scale"));
        GL_ASSERT(m_ratioUniform  = glGetUniformLocation(getID(), "ratio"));
    }

    virtual void update( double scale, double centerX, double centerY, double ratio) override
    {
        FloatFloat center[2] = { centerX, centerY };
        FloatFloat s(scale);
        static_assert( sizeof(center) == 4 * sizeof(float), "Size/align problem " );

        GL_ASSERT(glUniform4fv(m_centerUniform, 1, center[0].values));
        GL_ASSERT(glUniform2fv(m_scaleUniform, 1, s.values));
        GL_ASSERT(glUniform1f(m_ratioUniform, static_cast<GLfloat>(ratio)));
    }

    private:
    GLint m_centerUniform;
    GLint m_scaleUniform;
    GLint m_ratioUniform;
};

// global variables
const uint NUM_SHADERS = 2;

MandelbrotShader *shader = nullptr;
MandelbrotShader* shaders[NUM_SHADERS];
int shaderid = 0;

double scale    {2};        // zoom level
double centerX  {-0.5};     // center point x
double centerY  {0.0};      // center point y
double ratio    {1.0};      // aspect ratio

std::string float2hex( float x )
{
    static_assert(sizeof( float ) == sizeof( uint32 ), "`float` is not 32 bits wide");
    union { float f; uint32 i; } f2ui;
    char buf[sizeof( "0x00000000" )];
    f2ui.f = x;
    sprintf_s( buf, "0x%" PRIx32 , f2ui.i );
    return std::string( buf );
}

std::string double2hex( double x )
{
    static_assert(sizeof( double ) == sizeof( uint64 ), "`float` is not 32 bits wide");
    union { double d; uint64 i; } d2ui;
    char buf[sizeof( "0x0000000000000000" )];
    d2ui.d = x;
    sprintf_s( buf, "0x%" PRIx64, d2ui.i );
    return std::string( buf );
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

    GL_ASSERT( glDisable( GL_DEPTH_TEST ) );
    GL_ASSERT(glClampColor( GL_CLAMP_READ_COLOR, GL_FALSE));

    std::cout << "Renderer : " << glGetString( GL_RENDERER ) << std::endl;
    std::cout << "Vendor   : " << glGetString( GL_VENDOR ) << std::endl;
    std::cout << "OpenGL   : " << glGetString( GL_VERSION ) << std::endl;
    std::cout << "GLSL     : " << glGetString( GL_SHADING_LANGUAGE_VERSION ) <<std::endl;

    // build and compile our shader program
    // ------------------------------------
    SPMandelbrotShader shader_sp;
    DPMandelbrotShader shader_dp;

    shaders[0] = &shader_sp;
    shaders[1] = &shader_dp;

    shader = &shader_dp;
    shaderid = 1;

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
         1.0f,  1.0f, 0.0f,   // top right
         1.0f, -1.0f, 0.0f,  // bottom right
        -1.0f, -1.0f, 0.0f, // bottom left
        -1.0f,  1.0f, 0.0f,   // top left
    };
    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    unsigned int VBO, VAO, EBO;
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
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(4 * 3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    while (!glfwWindowShouldClose(window))
    {
        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // draw
        shader->useProgram();
        shader->update(scale, centerX, centerY, ratio);

        //glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        // glBindVertexArray(0); // no need to unbind it every time

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
    ratio = float(width) / float(height);
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
                centerY += (scale / sensitivity);
                break;
            }
            case GLFW_KEY_DOWN:
            {
                centerY -= (scale / sensitivity);
                break;
            }
            case GLFW_KEY_RIGHT:
            {
                centerX += (scale / sensitivity);
                break;
            }
            case GLFW_KEY_LEFT:
            {
                centerX -= (scale / sensitivity);
                break;
            }
            case GLFW_KEY_Z:
            {
                scale *= (1.0f - 1/sensitivity);
                break;
            }
            case GLFW_KEY_X:
            {
                scale *= (1.0f + 1/sensitivity);
                break;
            }
            case GLFW_KEY_P:
            {
                if (action == GLFW_PRESS)
                {
                    std::cout << centerX << " " << centerY << " " << scale << std::endl;
                }
                break;
            }
            case GLFW_KEY_S:
            {
                if (action == GLFW_PRESS)
                {
                    shaderid = (shaderid + 1) % NUM_SHADERS;
                    std::cout<<" switching to shader" << shaderid<<std::endl;
                    shader = shaders[shaderid];
                }
                break;
            }
        } // switch
    } // if (pressed)
}