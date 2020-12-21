cmake_minimum_required(VERSION 3.5)

add_subdirectory(PrototypePhysicsEngine)

add_library(RenderModule
	Camera.cpp
	Program.cpp
	RenderSystem.cpp
	Shape.cpp
	WindowManager.cpp
)

target_link_libraries(RenderModule PUBLIC
	../ext/GLFW/lib/glfw3.lib
	ComputeModule
	PhysicsModule
	OpenGLLoaderModule
)