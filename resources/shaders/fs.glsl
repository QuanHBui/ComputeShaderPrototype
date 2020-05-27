#version 430 core

in VS_OUT {
	vec3 fragNor;
} fs_in;

out vec4 color;

void main()
{
	color = vec4(fs_in.fragNor, 1.0f);
}