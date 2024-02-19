#pragma once

#include <spectr/render_gl/GraphicsApi.h>

namespace spectr::render_gl
{
class ImguiUtils
{
public:
    static void initImgui(GLFWwindow* window);

    static bool isCursorOverUI();
};
}
