#version 430 core
#extension GL_ARB_shader_storage_buffer_object : require

layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec4 vertNor;

void main()
{
	gl_Position = vertPos;
}