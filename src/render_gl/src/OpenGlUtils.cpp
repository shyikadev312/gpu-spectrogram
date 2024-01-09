#include <spectr/render_gl/OpenGlUtils.h>

#include <spectr/utils/Assert.h>
#include <spectr/utils/Exception.h>

namespace spectr::render_gl
{
constexpr size_t LogSize = 1024;

GLuint OpenGlUtils::createShaderProgram(const std::string& vertexShaderSource,
                                        const std::string& fragmentShaderSource)
{
    // compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char* vertexShaderSourcePtr = vertexShaderSource.c_str();
    glShaderSource(vertexShader, 1, &vertexShaderSourcePtr, NULL);
    glCompileShader(vertexShader);

    int success;
    char infoLog[LogSize];

    // check for shader compile errors
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, LogSize, NULL, infoLog);
        throw utils::Exception("Vertex shader compilation failed: {}", infoLog);
    }

    // compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragmentShaderSourcePtr = fragmentShaderSource.c_str();
    glShaderSource(fragmentShader, 1, &fragmentShaderSourcePtr, NULL);
    glCompileShader(fragmentShader);

    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, LogSize, NULL, infoLog);
        throw utils::Exception("Fragment shader compilation failed: {}", infoLog);
    }

    // link shaders
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, LogSize, NULL, infoLog);
        throw utils::Exception("Shader linking failed: {}", infoLog);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}
}
