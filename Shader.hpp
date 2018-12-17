#ifndef SHADER_HPP_
#define SHADER_HPP_

#include <CoreMacros.hpp>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <CoreGL.hpp>

/// class abstracting shader loading
class ShaderProgram
{
    public:
        ShaderProgram();

        void loadShaderStrings(const std::string& vsString, const std::string& psString);
        void loadShaderFiles ( const std::string& vsFileName, const std::string& psFileName);

        void useProgram() const;
        GLint getID() const {return m_shaderProgram;}

        ~ShaderProgram();
    private:
        void initialize( const char* vsStream, const char* psStream);

      private:
        GLint m_vertexShader;
        GLint m_pixelShader;
        GLint m_shaderProgram;


};

#endif // SHADER_HPP_