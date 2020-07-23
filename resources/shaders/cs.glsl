#version 430 core
#extension GL_ARB_compute_shader : require
#extension GL_ARB_shader_storage_buffer_object : require

precision highp float;

layout(local_size_x = 1, local_size_y = 2) in;

/**
 * @reference: 	Tomas Moller, "A Fast Triangle-Triangle Intersection Test"
 *				https://fileadmin.cs.lth.se/cs/Personal/Tomas_Akenine-Moller/pubs/tritri.pdf
 */

layout(std140, binding = 0) uniform transform_matrices
{
	mat4 projection;
	mat4 view;
	mat4 model_A;
	mat4 model_B;
};

layout(std430, binding = 1) readonly buffer ssbo_data
{
	vec4 positionBuffer_A[2763];
	vec4 positionBuffer_B[2763];
	uvec4 elementBuffer_A[5522];
	uvec4 elementBuffer_B[5522];
};

layout(std430, binding = 2) volatile buffer ssbo_color_out
{
	vec4 colorBuffer_A[2763];
	vec4 colorBuffer_B[2763];
};

layout(std430, binding = 3) writeonly buffer ssbo_debug
{
	vec4 debugOutput[];
};

#define EPSILON 0.0001f

#define ISECT(projVert0, projVert1, projVert2, distVert0, distVert1, distVert2, isectStart, isectEnd)	\
			isectStart = projVert0 + (projVert1 - projVert0) * distVert0/(distVert0 - distVert1);		\
			isectEnd = projVert0 + (projVert2 - projVert0) * distVert0/(distVert0 - distVert2);

#define SORT(a, b)			\
			if (a > b) {	\
				float c;	\
				c = a;		\
				a = b;		\
				b = c;		\
			}

// Test for intersection between coplanar triangles
bool coplanarTriTriTest(const vec3 v0, const vec3 v1, const vec3 v2,
						const vec3 u0, const vec3 u1, const vec3 u2,
						const vec3 N1)
{
	return true;
}

// Apparent there's no pointer or reference in GLSL
void computeIntersectInterval(	float projVert0, float projVert1, float projVert2,
								float distVert0, float distVert1, float distVert2,
								float prodDistVert0DistVert1, float prodDistVert0DistVert2,
								out float isectStart, out float isectEnd, out bool isCoplanar)
{
	// Check for which 2 edges are intersecting the plane by looking at the
	//  product of their vertices' signed distances.


	//=================Debug=====================
	// uint index_A = gl_GlobalInvocationID.x;
	// uvec3 tri_A = elementBuffer_A[index_A].xyz;
	// colorBuffer_A[tri_A.x].r = prodDistVert0DistVert1 - 1.0f;
	// colorBuffer_A[tri_A.x].g = prodDistVert0DistVert2 - 1.0f;
	// colorBuffer_A[tri_A.x].b = distVert1 - 1.0f;
	// colorBuffer_A[tri_A.x].a = projVert2 - 1.0f;
	//=================Debug=====================


	// Vert0 and Vert1 on the same side, look at edge Vert0...Vert2 and
	//  edge Vert1...Vert2
	if (prodDistVert0DistVert1 > 0.0f) {
		ISECT(projVert2, projVert0, projVert1, distVert2, distVert0, distVert1, isectStart, isectEnd);
	}
	// Vert0 and Vert2 on the same side, look at edge Vert0...Vert1 and
	//  edge Vert2...Vert1
	else if (prodDistVert0DistVert2 > 0.0f) {
		ISECT(projVert1, projVert0, projVert2, distVert1, distVert0, distVert2, isectStart, isectEnd);
	}
	// Vert1 and Vert2 on the same side, look at edge Vert1...Vert0 and
	//  edge Vert2...Vert0. Note that there's an extra check if Vert0 is
	//  in the plane.
	else if (distVert1*distVert2 > 0.0f || distVert0 != 0.0f) {
		ISECT(projVert0, projVert1, projVert2, distVert0, distVert1, distVert2, isectStart, isectEnd);
	}
	// At this point, Vert0 is in the plane.
	else if (distVert1 != 0.0f) {
		ISECT(projVert1, projVert0, projVert2, distVert1, distVert0, distVert2, isectStart, isectEnd);
	}
	// Both Vert0 and Vert1 are in the plane.
	else if (distVert2 != 0.0f) {
		ISECT(projVert2, projVert0, projVert1, distVert2, distVert0, distVert1, isectStart, isectEnd);
	}
	// Triangle is coplanar to the plane.
	else {
		isCoplanar = false;
	}
}

// Fast test for general 3D tri tri intersection. Does not return intersection segment
bool fastTriTriIntersect3DTest(	const vec3 v0, const vec3 v1, const vec3 v2,
								const vec3 u0, const vec3 u1, const vec3 u2)
{
	// 2 edges originating from v0 of the first triangle
	vec3 p1 = v0 - v1;
	vec3 p2 = v2 - v1;

	// Compute plane equation of first triangle
	vec3 N1 = cross(p1, p2);
	float d1 = -dot(N1, v1);

	// Signed distances of u0, u1, and u2 to plane of first triangle
	float distU0 = dot(N1, u0) + d1;
	float distU1 = dot(N1, u1) + d1;
	float distU2 = dot(N1, u2) + d1;


	//=========================DEBUG============================
	// uint index_A = gl_GlobalInvocationID.x;
	// uvec3 tri_A = elementBuffer_A[index_A].xyz;
	// colorBuffer_A[tri_A.y].r = distU0;
	// colorBuffer_A[tri_A.y].g = distU1;
	// colorBuffer_A[tri_A.y].b = distU2;
	// colorBuffer_A[tri_A.y].a = 1.0f;
	//=========================DEBUG============================


	// For coplanarity check later on. Using epsilon check because float precision
	if (abs(distU0) < EPSILON) distU0 = 0.0f;
	if (abs(distU1) < EPSILON) distU1 = 0.0f;
	if (abs(distU2) < EPSILON) distU2 = 0.0f;

	// If same sign and non-zero, no intersection. Early rejection
	float prodDistU0DistU1 = distU0 * distU1;
	float prodDistU0DistU2 = distU0 * distU2;
	if (prodDistU0DistU1 > 0.0f && prodDistU0DistU2 > 0.0f)
		return false;

	// 2 edges originating from u0 of the second triangle
	vec3 q1 = u0 - u1;
	vec3 q2 = u2 - u1;

	// Compute plane equation of second triangle
	vec3 N2 = cross(q1, q2);
	float d2 = -dot(N2, u1);

	// Signed distances of v0, v1, and v2 to plane of second triangle
	float distV0 = dot(N2, v0) + d2;
	float distV1 = dot(N2, v1) + d2;
	float distV2 = dot(N2, v2) + d2;


	//=========================DEBUG============================
	// uint index_A = gl_GlobalInvocationID.x;
	// uvec3 tri_A = elementBuffer_A[index_A].xyz;
	// colorBuffer_B[tri_A.x].r = distV0;
	// colorBuffer_B[tri_A.x].g = distV1;
	// colorBuffer_B[tri_A.x].b = distV2;
	// colorBuffer_B[tri_A.x].a = 1.0f;
	//=========================DEBUG============================


	// For coplanarity check later on. Using epsilon check because float precision
	if (abs(distV0) < EPSILON) distV0 = 0.0f;
	if (abs(distV1) < EPSILON) distV1 = 0.0f;
	if (abs(distV2) < EPSILON) distV2 = 0.0f;

	// If same sign and non-zero, no intersection. Early rejection
	float prodDistV0DistV1 = distV0 * distV1;
	float prodDistV0DistV2 = distV0 * distV2;
	if (prodDistV0DistV1 > 0.0f && prodDistV0DistV2 > 0.0f)
		return false;

	// Compute direction of intersection line
	vec3 intersectLineDirection = cross(N1, N2);

	// Optimization. Determine which component of intersectLineDirection is the max
	float maxComp = abs(intersectLineDirection.x);
	uint index = 0u;
	float yComp = abs(intersectLineDirection.y);
	float zComp = abs(intersectLineDirection.z);
	if (yComp > maxComp) maxComp = yComp, index = 1u;
	if (zComp > maxComp) maxComp = zComp, index = 2u;

	// Simplified projection of each vertex from triangle 1 and 2 onto intersectionLine L
	float projV0 = v0[index];
	float projV1 = v1[index];
	float projV2 = v2[index];

	float projU0 = u0[index];
	float projU1 = u1[index];
	float projU2 = u2[index];

	float isect0[2];
	float isect1[2];
	bool isCoplanar = false;


	//=========================DEBUG============================
	// uint index_A = gl_GlobalInvocationID.x;
	// uvec3 tri_A = elementBuffer_A[index_A].xyz;
	// colorBuffer_B[tri_A.x].r = prodDistV0DistV1;
	// colorBuffer_B[tri_A.x].g = prodDistV0DistV2;
	// colorBuffer_B[tri_A.x].b = prodDistU0DistU1;
	// colorBuffer_B[tri_A.x].a = prodDistU0DistU2;
	//=========================DEBUG============================


	// Compute intersection interval for triangle 1
	computeIntersectInterval(projV0, projV1, projV2, distV0, distV1, distV2,
							prodDistV0DistV1, prodDistV0DistV2,
							isect0[0], isect0[1], isCoplanar);

	// Compute intersection interval for triangle 2
	computeIntersectInterval(projU0, projU1, projU2, distU0, distU1, distU2,
							prodDistU0DistU1, prodDistU0DistU2,
							isect1[0], isect1[1], isCoplanar);

	// If the first triangle is coplanar, then the second should too, so
	//  perform this check only once.
	if (isCoplanar)
		return coplanarTriTriTest(v0, v1, v2, u0, u1, u2, N1);

	SORT(isect0[0], isect0[1]);
	SORT(isect1[0], isect1[1]);

	if (isect0[1] < isect1[0] || isect1[1] < isect0[0])
		return false;

	return true;
}

void main()
{
	uint index_A = gl_WorkGroupID.x;		// The way I set this up gl_GlobalInvocationID.x = gl_WorkGroupID.x
	uint index_B = gl_LocalInvocationID.y;

	uvec3 tri_A = elementBuffer_A[index_A].xyz;
	uvec3 tri_B = elementBuffer_B[index_B].xyz;

	// Prep the vertices, mapping them to world space
	vec3 v0 = (model_A * positionBuffer_A[tri_A.x]).xyz;
	vec3 v1 = (model_A * positionBuffer_A[tri_A.y]).xyz;
	vec3 v2 = (model_A * positionBuffer_A[tri_A.z]).xyz;

	vec3 u0 = (model_B * positionBuffer_B[tri_B.x]).xyz;
	vec3 u1 = (model_B * positionBuffer_B[tri_B.y]).xyz;
	vec3 u2 = (model_B * positionBuffer_B[tri_B.z]).xyz;

	bool collide = fastTriTriIntersect3DTest(v0, v1, v2, u0, u1, u2);

	// If collide load the color buffer A, the stanford bunny, with red color
	// Ignore colorBuffer B values, just for debugging
	if (collide) {
		colorBuffer_A[tri_A.x] = vec4(1.0f, 0.0f, 0.0f, 1.0f);
		colorBuffer_A[tri_A.y] = vec4(1.0f, 0.0f, 0.0f, 1.0f);
		colorBuffer_A[tri_A.z] = vec4(1.0f, 0.0f, 0.0f, 1.0f);

		colorBuffer_B[tri_B.x] = vec4(3.0f, 0.0f, 0.0f, 1.0f);
		colorBuffer_B[tri_B.y] = vec4(3.0f, 0.0f, 0.0f, 1.0f);
		colorBuffer_B[tri_B.z] = vec4(3.0f, 0.0f, 0.0f, 1.0f);
	} else {
		colorBuffer_A[tri_A.x] = vec4(0.0f, 0.0f, 0.0f, 1.0f);
		colorBuffer_A[tri_A.y] = vec4(0.0f, 0.0f, 0.0f, 1.0f);
		colorBuffer_A[tri_A.z] = vec4(0.0f, 0.0f, 0.0f, 1.0f);

		colorBuffer_B[tri_B.x] = vec4(2.0f, 0.0f, 0.0f, 1.0f);
		colorBuffer_B[tri_B.y] = vec4(2.0f, 0.0f, 0.0f, 1.0f);
		colorBuffer_B[tri_B.z] = vec4(2.0f, 0.0f, 0.0f, 1.0f);
	}

	// colorBuffer_A[tri_A.x] = vec4(view[0][0], view[1][0], view[2][0], view[3][0]);
	// colorBuffer_A[tri_A.y] = vec4(view[0][1], view[1][1], view[2][1], view[3][1]);
	// colorBuffer_A[tri_A.z] = vec4(view[0][2], view[1][2], view[2][2], view[3][2]);

	// colorBuffer_B[tri_B.x] = vec4(view[0][3], view[1][3], view[2][3], view[3][3]);
	// colorBuffer_B[tri_B.y] = vec4(3.0f, 0.0f, 0.0f, 1.0f);
	// colorBuffer_B[tri_B.z] = vec4(3.0f, 0.0f, 0.0f, 1.0f);


	// 	//========================DEBUG============================
	// 	// colorBuffer_A[index_A].r = sin(100.0f * u0.x);
	// 	// colorBuffer_A[index_A].g = sin(100.0f * u0.x);
	// 	// colorBuffer_A[index_A].b = sin(100.0f * u1.x);
	// 	// colorBuffer_A[index_A].a = sin(100.0f * u2.x);

	// 	// colorBuffer_B[index_A].r = sin(100.0f * u1.x);
	// 	// colorBuffer_B[index_A].g = sin(100.0f * u1.y);
	// 	// colorBuffer_B[index_A].b = sin(100.0f * u1.z);

	// 	// colorBuffer_B[index_A].r = positionBuffer_B[tri_B.x].x;
	// 	// colorBuffer_B[index_A].g = positionBuffer_B[tri_B.y].x;
	// 	// colorBuffer_B[index_A].b = positionBuffer_B[tri_B.z].x;
	// 	//========================DEBUG============================
	// }

}