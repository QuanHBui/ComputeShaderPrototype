cmake_minimum_required(VERSION 3.5)

# Apparently, top-level CMakeLists.txt must have a direct call to project() command.
project(prototype LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Control where the static and shared libraries are built
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

# Linking shared libraries on Windows is a pain in the backside.
#  Who would have guessed the ease of using Windows as development environment.
if(MSVC)
	set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
	set(CMAKE_SHARED_LIBS TRUE)
endif()

include_directories(
	src
	ext
	ext/glad/include
	ext/glm
	ext/GLFW/include
)

# When add_subdirectory is used, a CMakeLists.txt file is expected in
#  the specified directory.
add_subdirectory(ext)
add_subdirectory(src)
add_subdirectory(demos/Rigid-Body-Simulator)