#ifndef SHADER_HPP_
#define SHADER_HPP_

#include <CoreMacros.hpp>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <CoreGL.hpp>

/// Class abstracting shader loading. Only handles vertex and fragment shaders
class ShaderProgram
{
    public:
        /// Initialize shader handle (expect openGL context)
        ShaderProgram();

        /// Compile a shader from a given string
        void loadShaderStrings(const std::string& vsString, const std::string& psString);

        /// Compule a shader from given files (handles includes)
        void loadShaderFiles (const std::string& vsFileName, const std::string& psFileName);

        /// Calls glUseProgram() on the shader
        void useProgram() const;

        /// Returns the shader's handle
        GLint getID() const {return m_shaderProgram;}

        ~ShaderProgram();
    private:
        /// Common code for shader initialization
        void initialize(const std::string& vs, const std::string& ps);

        /// Helper function for shader #include
        static std::string preprocessIncludes(const std::string& shader, const std::string& filename, uint level = 0);

      private:
        // GL shader and program handles
        GLint m_vertexShader;
        GLint m_pixelShader;
        GLint m_shaderProgram;
};

#endif // SHADER_HPP_