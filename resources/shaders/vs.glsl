#version 430 core

layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec4 vertNor;
layout(location = 2) in uvec4 vertColor;

layout(std140, binding = 0) uniform transform_matrices
{
	mat4 projection;
	mat4 view;
};

uniform mat4 model;

out VS_OUT
{
	vec3 fragNor;
} vs_out;

void main()
{
	gl_Position = projection * view * model * vec4(vertPos.xyz, 1.0f);

	vs_out.fragNor = (model * vertPos).xyz;
}