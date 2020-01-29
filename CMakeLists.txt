# CMakeList.txt : CMake project for ComputeShaderPrototype, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.8)

project("ComputeShaderPrototype")

set(CMAKE_CXX_STANDARD_REQUIRED 17)
set(CMAKE_CXX_STANDARD 17)

# Use glob to get the list of all source files.
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Not needed, but nice to show include header and resource files to build in IDEs.
file(GLOB_RECURSE HEADERS "src/*.h")
file(GLOB_RECURSE GLSL "resources/*.glsl")

# Add source to this project's executable.
#add_executable(ComputeShaderPrototype "ComputeShaderPrototype.cpp" "ComputeShaderPrototype.h")
add_executable(${CMAKE_PROJECT_NAME} ${SOURCES} ${HEADERS} ${GLSL})



# Add GLFW
set(GLFW_DIR "$ENV{GLFW_DIR}")
if(GLFW_DIR)
	message(STATUS "GLFW environment variable found")

	option(GLFW_BUILD_EXAMPLE "GLFW_BUILD_EXAMPLES" OFF)
	option(GLFW_BUILD_TESTS "GLFW_BUILD_TESTS" OFF)
	option(GLFW_BUILD_DOCS "GLFW_BUILD_DOCS" OFF)
	if(CMAKE_BUILD_TYPE MATCHES Release)
		add_subdirectory(${GLFW_DIR} ${GLFW_DIR}/release)
	else()
		add_subdirectory(${GLFW_DIR} ${GLFW_DIR}/debug)
	endif()

	include_directories(${GLFW_DIR}/include)
	target_link_libraries(${CMAKE_PROJECT_NAME} glfw ${GLFW_LIBRARIES})
else()
	message(STATUS "GLFW environment variable `GLFW_DIR` not found, GLFW3 must be installed with the system")

	find_package(PkgConfig)
	if (PKGCONFIG_FOUND)
		message(STATUS "PkgConfig found")
		pkg_search_module(GLFW REQUIRED glfw3)
		include_directories(${GLFW_INLCUDE_DIRS})
		target_link_libraries(${CMAKE_PROJECT_NAME} ${GLFW_LIBRARIES})
	else() 
		message(STATUS "No PkgConfiq found")
		find_package(glfw3 REQUIRED)
		include_directories(${GLFW_INCLUDE_DIRS})
		target_link_libraries(${CMAKE_PROJECT_NAME} glfw)
	endif()
endif()



# Add GLM
# GLM is a header-only library -> add it to include directory
set(GLM_INCLUDE_DIR "$ENV{GLM_INCLUDE_DIR}")
if(GLM_INCLUDE_DIR)
	include_directories(${GLM_INCLUDE_DIR})
	message(STATUS "GLM environment variable found")
else()
	message(STATUS "GLM environment variable `GLM_INCLUDE_DIR` not found, GLM must be installed with the system")
endif()



# OS specific options and libraries
if(WIN32)
	target_link_libraries(${CMAKE_PROJECT_NAME} opengl32.lib)
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -Wall -pedantic")
	if(APPLE)
		# Required frameworks for GLFW
		target_link_libraries(${CMAKE_PROJECT_NAME} "-framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo")
	else()
		# For Linux
		target_link_libraries(${CMAKE_PROJECT_NAME} "GL" "dl")
	endif()
endif()