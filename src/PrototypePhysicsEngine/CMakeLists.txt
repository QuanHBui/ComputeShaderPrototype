cmake_minimum_required(VERSION 3.5)

add_library(ComputeModule SHARED
	../OpenGLUtils.cpp
	../GLSL.cpp
	ComputeProgram.cpp
)

target_link_libraries(ComputeModule PUBLIC
	OpenGLLoaderModule
)

add_library(PhysicsModule
	AtomicCounter.cpp
	P3BroadPhaseCollisionDetection.cpp
	P3Collider.cpp
	P3DynamicsWorld.cpp
	P3Epa.cpp
	P3Gjk.cpp
	P3NarrowPhaseCollisionDetection.cpp
)

target_link_libraries(PhysicsModule PUBLIC
	ComputeModule
)