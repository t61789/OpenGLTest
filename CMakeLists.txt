cmake_minimum_required(VERSION 3.15)  # 确保 CMake 版本足够新
	
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_SUPPRESS_REGENERATION TRUE)

project(OpenGLTest LANGUAGES C CXX)

file(GLOB_RECURSE SOURCES 
	src/*
)

include(E:/Programming/OpenGLTest/build/generators/conan_toolchain.cmake)

find_package(OpenGL REQUIRED)
find_package(GLFW3 REQUIRED)
find_package(assimp REQUIRED)
find_package(glm REQUIRED)
find_package(imgui REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(stb REQUIRED)

include_directories(src)
include_directories(lib/imgui)

add_executable(OpenGLTest 
	${SOURCES}
	lib/imgui/imgui_impl_glfw.cpp
	lib/imgui/imgui_impl_glfw.h
	lib/imgui/imgui_impl_opengl3.cpp
	lib/imgui/imgui_impl_opengl3.h
	lib/imgui/imgui_impl_opengl3_loader.h
)

# glad
include_directories(lib/glad/include)
target_link_directories(OpenGLTest PRIVATE "lib/glad/lib")

target_compile_definitions(OpenGLTest PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLEW)
target_link_libraries(
	OpenGLTest PRIVATE 
		glfw
		assimp::assimp
		glm::glm
		imgui::imgui
		nlohmann_json::nlohmann_json
		stb::stb
		glad
)

