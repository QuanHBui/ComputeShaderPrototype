#version 430
#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_storage_buffer_object : enable

layout(std430, binding = 0) volatile buffer shader_data
{
	vec4 dataA[1024];
	ivec4 dataB[1024];
};

layout(local_size_x = 1024, local_size_y = 1) in;
void main()
{
	uint index = gl_LocalInvocationID.x;
	vec3 data = dataA[index].xyz;
	dataB[index].x = int(data.x) * 3;
}