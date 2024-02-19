#include <spectr/render_gl/OpenGlUtils.h>

#include <spectr/utils/Assert.h>
#include <spectr/utils/Exception.h>

namespace spectr::render_gl
{
namespace
{
constexpr size_t ErrorLogSize = 1024;

GLuint compileShader(const std::vector<std::string>& sources, GLenum shaderType)
{
    const auto shaderId = glCreateShader(shaderType);

    std::vector<const GLchar*> sourcePointers;
    for (auto& source : sources)
    {
        sourcePointers.push_back(source.c_str());
    }

    glShaderSource(shaderId, static_cast<GLsizei>(sources.size()), sourcePointers.data(), NULL);
    glCompileShader(shaderId);

    int success;
    char infoLog[ErrorLogSize];
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shaderId, ErrorLogSize, NULL, infoLog);
        throw utils::Exception("Vertex shader compilation failed: {}", infoLog);
    }

    return shaderId;
}
}

GLuint OpenGlUtils::createShaderProgram(const std::string& vertexShaderSource,
                                        const std::string& fragmentShaderSource)
{
    return createShaderProgram(std::vector{ vertexShaderSource },
                               std::vector{ fragmentShaderSource });
}

GLuint OpenGlUtils::createShaderProgram(const std::vector<std::string>& vertexShaderSources,
                                        const std::vector<std::string>& fragmentShaderSources)
{
    const auto vertexShader = compileShader(vertexShaderSources, GL_VERTEX_SHADER);
    const auto fragmentShader = compileShader(fragmentShaderSources, GL_FRAGMENT_SHADER);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    char infoLog[ErrorLogSize];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, ErrorLogSize, NULL, infoLog);
        throw utils::Exception("Shader program linking failed: {}", infoLog);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}
}
