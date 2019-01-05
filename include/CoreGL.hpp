#pragma once
#ifndef CORE_GL_HPP_
#define CORE_GL_HPP_

/// Checks that an openGLContext is available (mostly for debug checks and asserts).
inline bool checkOpenGLContext()
{
    return glGetString( GL_VERSION ) != 0;
}

/// Gets the openGL error string (emulates gluErrorString())
inline const char* glErrorString(GLenum err)
{
    switch ( err )
    {
        case GL_INVALID_ENUM:
            return " Invalid enum : An unacceptable value is specified for an enumerated argument. The offending command is ignored and has no other side effect than to set the error flag.\n";
        case GL_INVALID_VALUE:
            return " Invalid value : A numeric argument is out of range. The offending command is ignored and has no other side effect than to set the error flag.\n";
        case GL_INVALID_OPERATION:
            return " Invalid operation : The specified operation is not allowed in the current state. The offending command is ignored and has no other side effect than to set the error flag.\n";
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return " Invalid framebuffer operation : The framebuffer object is not complete. The offending command is ignored and has no other side effect than to set the error flag.\n";
        case GL_OUT_OF_MEMORY:
            return " Out of memory : There is not enough memory left to execute the command. The state of the GL is undefined, except for the state of the error flags, after this error is recorded.\n";
        // case GL_STACK_UNDERFLOW:
        //    return " Stack underflow : An attempt has been made to perform an operation that would cause an internal stack to underflow.\n";
        //case GL_STACK_OVERFLOW:
        //    return " Stack overflow : An attempt has been made to perform an operation that would cause an internal stack to overflow.\n";
        case GL_NO_ERROR:
            return " No error\n";
        default:
            return " Unknown GL error\n";
    }
}

#ifdef CORE_ENABLE_ASSERT 
#define GL_ASSERT(x) \
    x; { \
        GLenum err = glGetError(); \
        if (err != GL_NO_ERROR) { \
            const char* errBuf = glErrorString(err); \
            std::cerr     << "OpenGL error (" << __FILE__ << ":" << __LINE__ \
                          << ", " << STRINGIFY(x) << ") : " << errBuf << "(" \
                          << err << " : 0x" << std::hex << err << std::dec << ")."; \
            BREAKPOINT(0);\
        } \
    }

/// This macro will query the last openGL error.
#define GL_CHECK_ERROR \
    {\
        GLenum err = glGetError(); \
        if (err != GL_NO_ERROR) { \
            const char* errBuf = glErrorString(err); \
            std::cerr     << "OpenGL error (" << __FILE__ << ":" << __LINE__ \
                          << ", glCheckError()) : " << errBuf << "(" \
                          << err << " : 0x" << std::hex << err << std::dec << ")."; \
            BREAKPOINT(0);\
        } \
    }

/// Ignore the previous openGL errors.
#define glFlushError() glGetError()

#else // Release version ignores the checks and errors.
#define GL_ASSERT(x) x
#define GL_CHECK_ERROR {}
#define glFlushError() {}
#endif // _DEBUG

#endif // CORE_GL_HPP_ 