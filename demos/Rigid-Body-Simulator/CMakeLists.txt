cmake_minimum_required(VERSION 3.5)

add_library(ApplicationModule Application.cpp)

# OS specific options and libraries
if(NOT WIN32)
	message(STATUS Adding GCC style compiler flags)
	add_compile_options(-Wall -pedantic -Werror=return-type)
endif()

# Set the executable.
add_executable(${CMAKE_PROJECT_NAME} main.cpp)

if (NOT WIN32)
	# TODO: The following linkings may be unneeded.
	if(APPLE)
		# Add required frameworks for GLFW.
		target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC "-framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo")
	else()
		# Link the Linux OpenGL library
		target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC "GL" "dl")
	endif()
else()
	# Link OpenGL on Windows
	target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC opengl32.lib)
endif()

target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC
	RenderModule
	PhysicsModule
	OpenGLLoaderModule
	ApplicationModule
)