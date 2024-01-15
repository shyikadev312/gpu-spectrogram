#include <spectr/calc_opencl/FftCooleyTukeyRadix2.h>
#include <spectr/calc_opencl/OpenclManager.h>
#include <spectr/calc_opencl/OpenclUtils.h>
#include <spectr/render_gl/GraphicsApi.h>
#include <spectr/utils/Exception.h>

#include <spdlog/spdlog.h>

using namespace spectr;

const float Eps = 1e-5f;

void AssertEpsEqual(float v1, float v2, float eps)
{
    if (std::abs(v1 - v2) > eps)
    {
        throw utils::Exception("{} and {} are not equal within epsilon {}", v1, v2, eps);
    }
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static void glfw_error_callback(int error, const char* desc)
{
    fputs(desc, stderr);
}

int main()
{
    spdlog::set_level(spdlog::level::debug);

    ASSERT(glfwInit());

    glfwSetErrorCallback(glfw_error_callback);

    GLFWwindow* window = glfwCreateWindow(640, 480, "My Title", NULL, NULL);
    ASSERT(window);

    glfwMakeContextCurrent(window);

    int version = gladLoadGL();
    ASSERT(version);

    glfwSetKeyCallback(window, key_callback);

    // initialization of OpenCL context from OpenGL context
    calc_opencl::OpenclManager openclManager;

    cl::Platform platform = openclManager.getPlatform();

#if defined(OS_WINDOWS)
    const std::vector<cl_context_properties> openclContextProperties{
        CL_GL_CONTEXT_KHR,
        reinterpret_cast<cl_context_properties>(glfwGetWGLContext(window)),
        CL_WGL_HDC_KHR,
        reinterpret_cast<cl_context_properties>(GetDC(glfwGetWin32Window(window))),
    };
#elif defined(OS_LINUX)
    const std::vector<cl_context_properties> openclContextProperties{
        CL_GL_CONTEXT_KHR,
        reinterpret_cast<cl_context_properties>(glfwGetGLXContext(window)),
        CL_GLX_DISPLAY_KHR,
        reinterpret_cast<cl_context_properties>(glfwGetX11Display()),
    };
#endif
    openclManager.initContext(openclContextProperties);
    auto context = openclManager.getContext();

    calc_opencl::OpenclUtils::printPlatformsAndDevices(std::cout);
    calc_opencl::OpenclUtils::printContextInfo(context, std::cout);

    const std::vector<float> realValues{ 1, 2, 3, 4 };
    calc_opencl::FftCooleyTukeyRadix2 fftOpenCl(openclManager.getContext(), realValues.size());
    fftOpenCl.execute(realValues);

    // create OpenGL buffer
    GLuint ssbo = 0;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(
      GL_SHADER_STORAGE_BUFFER, sizeof(float) * realValues.size(), nullptr, GL_DYNAMIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    cl::BufferGL openglOpenclBuffer(context, CL_MEM_READ_WRITE, ssbo);
    const auto v = fftOpenCl.getFffBufferCpu();

    glFinish();

    fftOpenCl.copyMagnitudesTo(openglOpenclBuffer, 0);

    //
    std::vector<float> outputRealValues(realValues.size(), 0);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glGetBufferSubData(
      GL_SHADER_STORAGE_BUFFER, 0, sizeof(float) * realValues.size(), outputRealValues.data());

    AssertEpsEqual(v[0].real(), 3, Eps);
    AssertEpsEqual(v[1].real(), -1, Eps);

    while (!glfwWindowShouldClose(window))
    {
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}
