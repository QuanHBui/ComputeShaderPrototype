#version 430 core

layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec4 vertNor;
layout(location = 2) in vec4 vertColor;

uniform mat4 projection, view, model;

out VS_OUT {
	vec3 fragNor;
	vec4 fragColor;
} vs_out;

void main()
{
	gl_Position = projection * view * model * vec4(vertPos.xyz, 1.0f);

	vs_out.fragNor = (model * vec4(vertNor.xyz, 0.0f)).xyz;
	vs_out.fragColor = vertColor;
}