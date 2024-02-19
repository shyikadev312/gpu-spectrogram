#pragma once

#include <spectr/render_gl/GraphicsApi.h>

#include <string>
#include <vector>

namespace spectr::render_gl
{
constexpr GLuint NoShaderProgram = 0;
constexpr GLuint NoBuffer = 0;
constexpr GLint NoUniform = -1;

class OpenGlUtils
{
public:
    static GLuint createShaderProgram(const std::string& vertexShaderSource,
                                      const std::string& fragmentShaderSource);

    static GLuint createShaderProgram(const std::vector<std::string>& vertexShaderSources,
                                      const std::vector<std::string>& fragmentShaderSources);
};
}
