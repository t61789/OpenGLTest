cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_SUPPRESS_REGENERATION TRUE)

project(OpenGLTest LANGUAGES C CXX)

file(GLOB_RECURSE SOURCES 
	src/*
)

add_executable(OpenGLTest ${SOURCES})
include_directories(src)

find_package(OpenGL REQUIRED)

# assimp
include_directories(lib/assimp/include)
target_link_directories(OpenGLTest PRIVATE "lib/assimp/lib")
configure_file(
    ${CMAKE_SOURCE_DIR}/lib/assimp/dll/assimp-vc143-mtd.dll
    ${CMAKE_BINARY_DIR}/assimp-vc143-mtd.dll
    COPYONLY
)

# glfw
include_directories(lib/glfw/include)
target_link_directories(OpenGLTest PRIVATE "lib/glfw/lib")
configure_file(
    ${CMAKE_SOURCE_DIR}/lib/glfw/dll/glfw3.dll
    ${CMAKE_BINARY_DIR}/glfw3.dll
    COPYONLY
)



# glm
include_directories(lib/glm)

# json
include_directories(lib/json)

# stbi_image
include_directories(lib/stbi_image)

# khr
include_directories(lib/khr)

# imgui
include_directories(lib/imgui/include)
target_link_directories(OpenGLTest PRIVATE "lib/imgui/lib")

# glad
include_directories(lib/glad/include)
target_link_directories(OpenGLTest PRIVATE "lib/glad/lib")


target_link_libraries(
	OpenGLTest PRIVATE 
	OpenGL::GL 
	glfw3 
	glfw3_mt 
	glfw3dll 
	assimp-vc143-mtd.lib
	glad
	imgui
)


