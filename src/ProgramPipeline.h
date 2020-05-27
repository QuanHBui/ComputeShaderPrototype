/**
 * ProgramPipeline is going to be a state machine with the following 4 states: 
 *  (1) Basic: contains a vertex program and a fragment program
 *  (2) Compute: vertex + fragment + compute
 *  (3) Geomentry: vertex + geometry + fragment + compute
 *  (4) Barebone compute: compute ONLY
 * 
 * The state of program pipeline can be set explicitly when created. Or it would
 *  dynamic switch states depending on what kind of shader stages attaching to it
 * 
 * @author: Quan BUi
 * @version: 04/05/2020
 */

#pragma once

#ifndef PROGRAM_PIPELINE_H
#define PROGRAM_PIPELINE_H

#include <glad/glad.h>

class ProgramPipeline 
{
private:
	GLuint pipelineHandle = 0u;
	GLuint vertexHandle = 0u, fragmentHandle = 0u, 
		computeHandle = 0u, GeometryHandle = 0u;

public:
	~ProgramPipeline();

	bool init();

	// To change shader code of each shader stage. This is to allow 
	//  mix-and-match shaders after linking.
	// If a set function is called for a shader stage that hasn't been
	//  in the pipeline, an add call will be invoked instead.
	void setVertexProgram();
	void setFragmentProgram();
	void setGeometryProgram();
	void setComputeProgram();

	// To add an additional shader stage
	bool addVertexProgram();
	bool addFragmentProgram();
	bool addGeometryProgram();
	bool addComputeProgram();

	void bind();
	void unbind();
};

#endif // PROGRAM_PIPELINE_H