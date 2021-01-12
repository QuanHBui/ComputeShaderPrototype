#version 430 core

out vec4 color;

uniform vec3 vertColor;

void main()
{
	color = vec4(vertColor, 1.0f);
}
