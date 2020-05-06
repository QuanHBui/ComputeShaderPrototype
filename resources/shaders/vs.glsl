#version 430 core

layout(location = 0) in vec4 vertPos;
layout(location = 2) in vec4 vertNor;

uniform TransformMatrixBlock
{
	mat4 projection;
	mat4 view;
	mat4 model;
} transformMatrices;

void main()
{
	gl_Position = vertPos;
}