#version 430 core

in VS_OUT {
	vec3 fragNor;
	vec4 fragColor;
} fs_in;

out vec4 color;

void main()
{
	if (fs_in.fragColor.r == 0.0f)
		color = vec4(fs_in.fragNor, 1.0f);
	else
		color = vec4(vec3(fs_in.fragColor.r, 0.0f, 0.0f), 1.0f);
}