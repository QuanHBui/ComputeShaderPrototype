#version 430 core

layout(location = 0) in vec4 vertPos;

uniform mat4 projection, view;

void main()
{
	gl_PointSize = 10.0f;
	gl_Position = projection * view * vec4(vertPos.xyz, 1.0f);
}
