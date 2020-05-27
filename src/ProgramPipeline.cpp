#include "ProgramPipeline.h"

#include "Program.h"
#include "GLSL.h"

ProgramPipeline::~ProgramPipeline()
{
	CHECKED_GL_CALL(glDeleteProgramPipelines(1, &pipelineHandle));
}

bool ProgramPipeline::init() 
{
	glGenProgramPipelines(1, &pipelineHandle);

	return true;
}

void ProgramPipeline::bind() 
{
	CHECKED_GL_CALL(glBindProgramPipeline(pipelineHandle));
}

void ProgramPipeline::unbind()
{
	CHECKED_GL_CALL(glBindProgramPipeline(0));
}