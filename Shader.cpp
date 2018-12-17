#include "Shader.hpp"

#include <fstream>
#include <sstream>

ShaderProgram::ShaderProgram()
{
    GL_ASSERT(m_vertexShader = glCreateShader(GL_VERTEX_SHADER));
    GL_ASSERT(m_pixelShader = glCreateShader(GL_FRAGMENT_SHADER));
    GL_ASSERT(m_shaderProgram = glCreateProgram());
}

void ShaderProgram::loadShaderStrings(const std::string &vsString, const std::string &psString)
{
    initialize(vsString.c_str(), psString.c_str());
}

void ShaderProgram::loadShaderFiles(const std::string &vsFileName, const std::string &psFileName)
{

    std::ifstream vss(vsFileName);
    CORE_ASSERT(vss.good(), "Error loading vertex shader file " << vsFileName);

    std::stringstream vsstr;
    vsstr << vss.rdbuf();
    std::string vs(vsstr.str());

    std::ifstream pss(psFileName);
    CORE_ASSERT(pss.good(), "Error loading pixel shader file " << psFileName);

    std::stringstream psstr;
    psstr << pss.rdbuf();
    std::string ps(psstr.str());

    initialize(vs.c_str(), ps.c_str());
}

void checkCompilation(GLuint shader, const std::string &errorStr)
{
    int success;
    char log[512];
    GL_ASSERT(glGetShaderiv(shader, GL_COMPILE_STATUS, &success));
    if (!success)
    {
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "Error : " << errorStr << "\n"
                  << log << std::endl;
        BREAKPOINT(0);
    }
}
void checkLink(GLuint shader, const std::string &errorStr)
{
    int success;
    char log[512];
    GL_ASSERT(glGetProgramiv(shader, GL_LINK_STATUS, &success));
    if (!success)
    {
        glGetProgramInfoLog(shader, 512, nullptr, log);
        std::cerr << "Error : " << errorStr << "\n"
                  << log << std::endl;
        BREAKPOINT(0);
    }
}

void ShaderProgram::initialize(const char *vsStream, const char *psStream)
{
    // Load vertex shader
    GL_ASSERT(glShaderSource(m_vertexShader, 1, &vsStream, nullptr));
    GL_ASSERT(glCompileShader(m_vertexShader));
    checkCompilation(m_vertexShader, "Vertex shader compilation");

    // Load fragment shader
    GL_ASSERT(glShaderSource(m_pixelShader, 1, &psStream, nullptr));
    GL_ASSERT(glCompileShader(m_pixelShader));
    checkCompilation(m_pixelShader, "Pixel shader compilation");

    // Link shaders
    GL_ASSERT(glAttachShader(m_shaderProgram, m_vertexShader));
    GL_ASSERT(glAttachShader(m_shaderProgram, m_pixelShader));
    GL_ASSERT(glLinkProgram(m_shaderProgram));
    checkLink(m_shaderProgram, "Shader program link");
}
void ShaderProgram::useProgram() const
{
    GL_ASSERT(glUseProgram(m_shaderProgram));
}

ShaderProgram::~ShaderProgram()
{
    glDeleteShader(m_vertexShader);
    glDeleteShader(m_pixelShader);
    glDeleteProgram(m_shaderProgram);
}