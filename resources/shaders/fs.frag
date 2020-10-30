#version 430 core

in VS_OUT {
	vec3 fragNor;
	vec4 fragColor;
} fs_in;

uniform uint redOrNo;

out vec4 color;

void main()
{
	if (redOrNo == 1)
		color = vec4(0.8f, 0.0f, 0.0f, 1.0f);
	else
		color = vec4(0.0f, 0.8f, 0.0f, 1.0f);
}