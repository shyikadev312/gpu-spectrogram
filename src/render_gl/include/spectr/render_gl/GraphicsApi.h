#pragma once

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#elif defined(OS_LINUX)
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_GLX
#else
#error "Unsupported OpenGL platform."
#endif

#define GLM_GTC_matrix_transform 1

#include <glad/glad.h>
#include <GL/glext.h>

#ifdef _WIN32
# include <backends/imgui_impl_glfw.h>
# include <backends/imgui_impl_opengl3.h>
#else
# include "../res/bindings/imgui_impl_glfw.h"
# include "../res/bindings/imgui_impl_opengl3.h"
#endif
#include <imgui.h>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#undef None

namespace spectr::render_gl
{
using Color = glm::vec4;

constexpr Color makeColor(int r, int g, int b, int a = 255)
{
    return { r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f };
}

struct Range
{
    float min;
    float max;

    inline operator glm::vec2() const { return { min, max }; }
};
}
