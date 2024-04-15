/*
 * This program executes and compares the results of FFT CPU and OpenCL implementations. It's useful
 * for debugging.
 */

#include <spectr/audio_loader/SignalDataGenerator.h>
#include <spectr/calc_cpu/FftCooleyTukeyRadix2.h>
#include <spectr/calc_opencl/FftCooleyTukeyRadix2CL.h>
#include <spectr/calc_opencl/OpenclManager.h>
#include <spectr/calc_opencl/OpenclUtils.h>
#include <spectr/utils/Exception.h>

#include <glad/glad.h>

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

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <algorithm>
#include <iomanip>

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

template<typename Tfrom, typename Tto>
std::vector<Tto> convertVector(const std::vector<Tfrom>& src)
{
    std::vector<Tto> dst;
    std::transform(src.begin(),
                   src.end(),
                   std::back_inserter(dst),
                   [](const Tfrom& val) { return static_cast<Tto>(val); });
    return dst;
}

int main()
{
    ASSERT(glfwInit());

    glfwSetErrorCallback(glfw_error_callback);

    GLFWwindow* window = glfwCreateWindow(640, 480, "My Title", NULL, NULL);
    ASSERT(window);

    glfwMakeContextCurrent(window);

    int version = gladLoadGL();
    ASSERT(version);

    glfwSetKeyCallback(window, key_callback);

    // initialization of OpenCL context from OpenGL context
#ifdef _WIN32
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

    calc_opencl::OpenclManager openclManager(openclContextProperties);
    calc_opencl::OpenclUtils::printPlatformsAndDevices(std::cout);

    auto context = openclManager.getContext();
    calc_opencl::OpenclUtils::printContextInfo(context, std::cout);

    // const auto audioData = audio_loader::AudioDataGenerator::generate(16, 1, { { 4 } });
    // const auto audioData = audio_loader::AudioDataGenerator::generate(1, 64, { { 0.25f } });
    // const auto audioData = audio_loader::AudioDataGenerator::generate(8, 1, { { 2.31f } });

    const auto audioData = audio_loader::SignalDataGenerator::generate<float>(1024, 2, { { 4 } });
    const auto realValues = audioData.getSampleDataFloat(0);

    // const std::vector<float> realValues{ 1, 2, 3, 4, 5, 6, 7, 8 };
    calc_opencl::FftCooleyTukeyRadix2 fftOpenCl(openclManager.getContext(), realValues.size());
    fftOpenCl.execute(realValues);

    const auto fftCpu = calc_cpu::FftCooleyTukeyRadix2::getFFT(realValues);
    const auto fftGpu = fftOpenCl.getFffBufferCpu();

    const auto magnitudesCpu = calc_cpu::FftCooleyTukeyRadix2::getMagnitudesFromFFT(fftCpu);
    const auto magnitudesGpu = calc_cpu ::FftCooleyTukeyRadix2::getMagnitudesFromFFT(fftGpu);

    for (size_t i = 0; i < realValues.size(); ++i)
    {
        bool isEqual = true;
        constexpr auto Eps = 1e-3;

        const auto cpuValue = magnitudesCpu[i];
        const auto gpuValue = magnitudesGpu[i];

        if (std::abs(cpuValue - gpuValue) > Eps)
        {
            isEqual = false;
            // DebugBreak();
        }

        constexpr auto Precision = 5;

        std::cout << "#" << i << "\n";
        std::cout << "CPU: " << std::fixed << std::setprecision(Precision) << cpuValue
                  << (isEqual ? "" : "<----") << "\n";
        std::cout << "GPU: " << std::fixed << std::setprecision(Precision) << gpuValue << "\n";
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}
