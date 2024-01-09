#include <glad/glad.h>

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

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <spectr/calc_opencl/FftCooleyTukeyRadix2.h>
#include <spectr/calc_opencl/OpenclManager.h>
#include <spectr/calc_opencl/OpenclUtils.h>
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

int main()
{
    spdlog::set_level(spdlog::level::debug);

    ASSERT(glfwInit());

    GLFWwindow* window = glfwCreateWindow(640, 480, "My Title", NULL, NULL);
    ASSERT(window);

    glfwMakeContextCurrent(window);

    int version = gladLoadGL();
    ASSERT(version);

    glfwSetKeyCallback(window, key_callback);

    // initialization of OpenCL context from OpenGL context
    calc_opencl::OpenclManager openclManager;

    cl::Platform platform = openclManager.getPlatform();

    const std::vector<cl_context_properties> contextProperties{
        CL_GL_CONTEXT_KHR,
        reinterpret_cast<cl_context_properties>(glfwGetWGLContext(window)),
        CL_WGL_HDC_KHR,
        reinterpret_cast<cl_context_properties>(GetDC(glfwGetWin32Window(window))),
    };

    openclManager.initContext(contextProperties);
    auto context = openclManager.getContext();

    calc_opencl::OpenclUtils::printContextInfo(context, std::cout);

    const std::vector<float> realValues{ 1, 2 };

    // create OpenGL buffer
    GLuint inputBuffer = 0;
    glCreateBuffers(1, &inputBuffer);
    glBindBuffer(GL_UNIFORM_BUFFER, inputBuffer);
    glBufferData(GL_UNIFORM_BUFFER, realValues.size(), realValues.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    cl::BufferGL inputBufferCl(context, CL_MEM_READ_ONLY, inputBuffer);

    calc_opencl::FftCooleyTukeyRadix2 fftOpenCl(openclManager.getContext(), realValues.size());

    // const auto v = fftOpenCl.execute(inputBufferCl);
    fftOpenCl.execute({ 1, 2 });
    const auto v = fftOpenCl.getFinalDataBufferCpu();

    AssertEpsEqual(v[0], 3, Eps);
    AssertEpsEqual(v[1], -1, Eps);

    while (!glfwWindowShouldClose(window))
    {
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}
