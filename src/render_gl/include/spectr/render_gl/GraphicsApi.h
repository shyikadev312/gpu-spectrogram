#pragma once

#if defined(OS_WINDOWS)
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#elif defined(OS_LINUX)
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_GLX
#elif
#error "Unsupported OpenGL platform."
#endif

#define GLM_GTC_matrix_transform 1

// #define IMGUI_IMPL_OPENGL_LOADER_CUSTOM 1

#include <glad/glad.h>

#include "../res/bindings/imgui_impl_glfw.h"
#include "../res/bindings/imgui_impl_opengl3.h"
#include <imgui.h>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#undef None

namespace spectr::render_gl
{
using Color = glm::vec4;
}
