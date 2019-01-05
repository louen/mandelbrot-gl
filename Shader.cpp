#include "Shader.hpp"

#include <fstream>
#include <sstream>
#include <regex>

// Helper functions
namespace
{
// Load a file to a string
std::string loadFile(const std::string &fileName)
{
    std::ifstream iss(fileName);
    CORE_ASSERT(iss.good(), "Error loading file " << fileName);
    std::stringstream sstr;
    sstr << iss.rdbuf();
    return sstr.str();
}

// Print a file on stdout with line nunmbers ( useful for shader compilation error )
void printWithLines(const std::string& fileStr)
{
    std::stringstream ss(fileStr);
    uint linenum = 1;
    std::string line;
    while (std::getline(ss, line))
    {
        if (line.substr(0,5) != "#line")
        {
            std::cout << linenum << "\t" << line << std::endl;
            ++linenum;
        }
        else
        {
            std::string strnum = line.substr(6);
            linenum = std::stoi(strnum);
        }
    }
}

// Commpile a shader, report errors
void compile(GLuint shader, const std::string& shaderStr, const std::string &errorStr)
{
    // Compile
    const char* str = shaderStr.c_str();
    GL_ASSERT(glShaderSource(shader, 1, &str, nullptr));
    GL_ASSERT(glCompileShader(shader));

    // Check for errors
    int success;
    char log[512];
    GL_ASSERT(glGetShaderiv(shader, GL_COMPILE_STATUS, &success));
    if (!success)
    {
        printWithLines(shaderStr);
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "Error : " << errorStr << "\n"
                  << log << std::endl;
        BREAKPOINT(0);
    }
}

// Link a vertex and a pixel shader to a shader program, report errors
void link(GLuint vsID, GLuint psID, GLuint shaderID, const std::string &errorStr)
{

    GL_ASSERT(glAttachShader(shaderID, vsID));
    GL_ASSERT(glAttachShader(shaderID, psID));
    GL_ASSERT(glLinkProgram(shaderID));
    int success;
    char log[512];
    GL_ASSERT(glGetProgramiv(shaderID, GL_LINK_STATUS, &success));
    if (!success)
    {
        glGetProgramInfoLog(shaderID, 512, nullptr, log);
        std::cerr << "Error : " << errorStr << "\n"
                  << log << std::endl;
        BREAKPOINT(0);
    }
}

} // namespace

// Init shader variables
ShaderProgram::ShaderProgram()
{
    GL_ASSERT(m_vertexShader = glCreateShader(GL_VERTEX_SHADER));
    GL_ASSERT(m_pixelShader = glCreateShader(GL_FRAGMENT_SHADER));
    GL_ASSERT(m_shaderProgram = glCreateProgram());
}

// Direct string loading
void ShaderProgram::loadShaderStrings(const std::string &vsString, const std::string &psString)
{
    initialize(vsString, psString);
}

// Recursive function to load shader includes
std::string ShaderProgram::preprocessIncludes(const std::string &shader, const std::string &filename, uint level /*= 0*/)
{
    CORE_ERROR_IF(level > 32, "Header inclusion depth limit reached, might be caused by cyclic header inclusion");

    static const std::regex re("^[ ]*#[ ]*include[ ]+[\"<](.*)[\">].*");
    std::stringstream input;
    std::stringstream output;
    input << shader;

    size_t line_number = 1;
    std::smatch matches;

    std::string line;
    while (std::getline(input, line))
    {
        if (std::regex_search(line, matches, re))
        {
            std::string include_file = matches[1];
            std::string include_string;

            include_string = loadFile(include_file);
            output << preprocessIncludes(include_string, filename, level + 1) << std::endl;
        }
        else
        {
            if (line.find("#version") == std::string::npos)
            {
                //output << "#line "<< line_number << "  " << filename << std::endl;
                output << "#line "<< line_number << std::endl;
            }
            output << line << std::endl;
        }
        ++line_number;
    }
    return output.str();
}

// Load shaders from files
void ShaderProgram::loadShaderFiles(const std::string &vsFileName, const std::string &psFileName)
{
    std::string vs = preprocessIncludes(loadFile(vsFileName), vsFileName);
    std::string ps = preprocessIncludes(loadFile(psFileName), psFileName);

    initialize(vs, ps);
}

void ShaderProgram::initialize(const std::string& vs, const std::string& ps)
{
    // Load vertex shader
    compile(m_vertexShader, vs, "Vertex shader compilation");

    // Load fragment shader
    compile(m_pixelShader, ps, "Pixel shader compilation");

    // Link shaders
    link(m_vertexShader, m_pixelShader, m_shaderProgram, "Shader program link");
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