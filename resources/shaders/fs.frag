#version 430 core

in VS_OUT {
	vec3 fragNor;
	vec4 fragColor;
} fs_in;

uniform uint redOrNo;
uniform uint isRigid;

out vec4 color;

void main()
{
	if (isRigid == 1)
		color = vec4(0.0f, 0.6f, 0.0f, 0.6f);
	else
		color = vec4(0.0f, 0.1f, 0.9f, 0.6f);

	if (redOrNo == 1)
		color = vec4(0.6f, 0.0f, 0.0f, 0.6f);
}
