cmake_minimum_required(VERSION 3.25)
project(Spectr VERSION 0.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set binary/executables output dir
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Set OS macro
if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_compile_definitions(OS_WINDOWS)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    add_compile_definitions(OS_LINUX)
else()
    message(FATAL_ERROR "Platform: \'${CMAKE_SYSTEM_NAME}\' is an unsupported OS platform.")
endif()

# If this option is ON, it will force enable assertions (even in the release build)
option(FORCE_ENABLE_ASSERTIONS OFF)
if (FORCE_ENABLE_ASSERTIONS)
    message(STATUS "Force enabled assertions.")
    add_compile_definitions(FORCE_ENABLE_ASSERTIONS)
endif()

# Conan initialization
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
  message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
  file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/0.18.1/conan.cmake"
                "${CMAKE_BINARY_DIR}/conan.cmake"
                TLS_VERIFY ON)
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_cmake_autodetect(settings)

conan_cmake_install(PATH_OR_REFERENCE ${PROJECT_SOURCE_DIR}
                    BUILD missing
                    REMOTE conancenter
                    SETTINGS ${settings})

# Third party dependencies
find_package(Boost REQUIRED)
find_package(fmt REQUIRED)
find_package(glm REQUIRED)
find_package(spdlog REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glad REQUIRED)
find_package(GTest REQUIRED)
find_package(imgui REQUIRED)
find_package(benchmark REQUIRED)
find_package(OpenCL REQUIRED)
find_package(OpenCLHeaders REQUIRED)
find_package(OpenCLHeadersCpp REQUIRED)

# Adding ImGui platform specific source files - it is designed to be done in a strange way
add_library(spectr.imgui_my_lib STATIC
    ${CMAKE_CURRENT_BINARY_DIR}/imgui_bindings/imgui_impl_glfw.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/imgui_bindings/imgui_impl_opengl3.cpp
)
target_include_directories(spectr.imgui_my_lib PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}/imgui_bindings/
)
target_link_libraries(spectr.imgui_my_lib imgui::imgui glad::glad glfw)

enable_testing()

add_subdirectory(src)
