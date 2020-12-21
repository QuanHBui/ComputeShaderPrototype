cmake_minimum_required(VERSION 3.5)

project(external_dependencies LANGUAGES C)

add_library(OpenGLLoaderModule SHARED glad/src/glad.c)

target_include_directories(OpenGLLoaderModule PUBLIC
	glad/include
)