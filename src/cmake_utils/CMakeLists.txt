#==============================================================================
macro(DEFINE_MODULE ModuleName)

add_library(${ModuleName} STATIC "")

target_include_directories(${ModuleName} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${ModuleName} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

file (GLOB_RECURSE COLLECTED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
target_sources(${ModuleName} PRIVATE
	${COLLECTED_SOURCES})

target_link_libraries(${ModuleName}
	fmt::fmt
	glm::glm
	glfw
	glad::glad
	spdlog::spdlog
	imgui::imgui
	spectr.imgui_my_lib
	# Boost::program_options
)

endmacro()

#==============================================================================
macro(DEFINE_EXECUTABLE_MODULE ModuleName)

add_executable(${ModuleName} "")

target_include_directories(${ModuleName} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(${ModuleName} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${ModuleName} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# file (GLOB_RECURSE COLLECTED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file (GLOB_RECURSE COLLECTED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_sources(${ModuleName} PRIVATE
	${COLLECTED_SOURCES})

target_link_libraries(${ModuleName}
	fmt::fmt
	glm::glm
	glfw
	glad::glad
	spdlog::spdlog
	imgui::imgui
	spectr.imgui_my_lib 
	# Boost::program_options
)

endmacro()


#==============================================================================
macro(DEFINE_TEST_MODULE TargetName)

set (TestTargetName "${TargetName}.unittest")

add_executable(${TestTargetName} "")

file (GLOB_RECURSE COLLECTED_SOURCES "*.cpp")
target_sources(${TestTargetName} PRIVATE 
	${COLLECTED_SOURCES}
)

target_include_directories(${TestTargetName} PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${TestTargetName}
	${TargetName}
)

target_link_libraries(${TestTargetName}
	# test_utils
	GTest::gtest_main
	GTest::gmock
)

enable_testing()
add_test(NAME "${TestTargetName}Program"
	COMMAND ${TestTargetName}
)

create_symlink_to("assets")

endmacro()

#==============================================================================
# Create a symlink in the output directory to some directory in the source tree.
function(create_symlink_to sourceDirPath)

file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
file(CREATE_LINK
    ${PROJECT_SOURCE_DIR}/${sourceDirPath}
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${sourceDirPath}
    SYMBOLIC)

endfunction()

#==============================================================================
