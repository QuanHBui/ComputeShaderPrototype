#include "P3OpenGLComputeBroadPhase.h"

#include "ComputeProgram.h"
#include "GLSL.h"

void P3OpenGLComputeBroadPhase::init()
{
	initShaderPrograms();
	initGpuBuffers();
}

CollisionPairGpuPackage const &P3OpenGLComputeBroadPhase::step(std::vector<P3BoxCollider> const &boxColliders)
{
	detectCollisionPairs(boxColliders);

	return *mpCollisionPairCpuData;
}

/**
 * Maybe hard reset on everything
 */
void P3OpenGLComputeBroadPhase::reset()
{
	mAtomicCounter.reset();
}

void P3OpenGLComputeBroadPhase::initShaderPrograms()
{
	mComputeProgramIDContainer[P3_UPDATE_AABBS]  = createComputeProgram("../resources/shaders/updateAABBs.comp");
	mComputeProgramIDContainer[P3_ODD_EVEN_SORT] = createComputeProgram("../resources/shaders/evenOddSort.comp");
	mComputeProgramIDContainer[P3_SAP]           = createComputeProgram("../resources/shaders/sap.comp");
	// mComputeProgramIDContainer[P3_ASSIGN_MORTON_CODES] = createComputeProgram("../resources/shaders/assignMortonCodes.comp");
	// mComputeProgramIDContainer[P3_BUILD_PARALLEL_LINEAR_BVH] = createComputeProgram("../resources/shaders/buildParallelLinearBVH.comp");
	// mComputeProgramIDContainer[P3_SORT_LEAF_NODES] = createComputeProgram("../resources/shaders/sortLeafNodes.comp");
}

GLuint P3OpenGLComputeBroadPhase::initGpuBuffers()
{
	mAtomicCounter.init();

	glGenBuffers(cBroadPhaseSsboCount, mSsboIDs.data());

	GLbitfield mapFlags = GL_MAP_WRITE_BIT
						| GL_MAP_PERSISTENT_BIT // Keep being mapped while drawing/computing
						| GL_MAP_COHERENT_BIT;  // Writes are automatically visible to GPU

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSsboIDs[P3_BOX_COLLIDERS]);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(BoxColliderGpuPackage), nullptr, mapFlags);

	// Keep it mapped until end of program
	mpBoxColliderCpuData = static_cast<BoxColliderGpuPackage *>(glMapBufferRange(
		GL_SHADER_STORAGE_BUFFER,
		0,
		sizeof(BoxColliderGpuPackage),
		mapFlags ));

	mapFlags = GL_MAP_READ_BIT
			 | GL_MAP_PERSISTENT_BIT
			 | GL_MAP_COHERENT_BIT;

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSsboIDs[P3_COLLISION_PAIRS]);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(CollisionPairGpuPackage), nullptr, mapFlags);

	mpCollisionPairCpuData = static_cast<CollisionPairGpuPackage *>(glMapBufferRange(
		GL_SHADER_STORAGE_BUFFER,
		0,
		sizeof(CollisionPairGpuPackage),
		mapFlags
	));

	glGenBuffers(1, &mDispatchIndirectBufferID);
	glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, mDispatchIndirectBufferID);
	glBufferStorage(GL_DISPATCH_INDIRECT_BUFFER, sizeof(DispatchIndirectCommand), &mDispatchIndirectCommand, 0);

	return mDispatchIndirectBufferID;
}

void P3OpenGLComputeBroadPhase::buildBvhTreeOnGpu()
{

}

void P3OpenGLComputeBroadPhase::detectCollisionPairs(std::vector<P3BoxCollider> const &boxColliders)
{
	// Pack data for GPU
	for (unsigned int i = 0; i < boxColliders.size(); ++i)
	{
		for (unsigned int j = 0; j < cBoxColliderVertCount; ++j)
		{
			// This buffer is being persistently mapped. TODO: No sync or triple buffering being used yet.
			mpBoxColliderCpuData->boxColliders[i][j] = boxColliders[i].mVertices[j];
		}
	}

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSsboIDs[P3_AABBS]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(AabbGpuPackage), nullptr, GL_DYNAMIC_COPY);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0u, mSsboIDs[P3_BOX_COLLIDERS]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1u, mSsboIDs[P3_AABBS]);

	//================== Start of update AABBs ==================//
	GLuint currProgID = mComputeProgramIDContainer[P3_UPDATE_AABBS];
	glUseProgram(currProgID);

	GLuint uniformIdx = glGetUniformLocation(currProgID, "currNumColliders");
	glUniform1ui(uniformIdx, boxColliders.size());

	glDispatchComputeIndirect(0);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	//================ End of update AABBs ================//

	//================== Start of SAP ==================//

	// SORT ON X-AXIS
	currProgID = mComputeProgramIDContainer[P3_ODD_EVEN_SORT];
	glUseProgram(currProgID);

	glUniform1ui(uniformIdx, boxColliders.size());
	uniformIdx = glGetUniformLocation(currProgID, "evenOrOdd");

	GLuint subroutineIdx = glGetSubroutineIndex(currProgID, GL_COMPUTE_SHADER, "sortX");
	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &subroutineIdx);

	int iterations = boxColliders.size();
	while (iterations--)
	{
		//--------------------------- Dispatch and synchronize ---------------------------//
		// -1 for even, 1 for odd; we start processing the odd pair first
		glUniform1i(uniformIdx, 1);
		glDispatchComputeIndirect(0);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glUniform1i(uniformIdx, -1);
		glDispatchComputeIndirect(0);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	// SWEEP AND PRUNE ON X-AXIS
	currProgID = mComputeProgramIDContainer[P3_SAP];
	glUseProgram(currProgID);

	mAtomicCounter.bindTo(0u);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2u, mSsboIDs[P3_COLLISION_PAIRS]);

	uniformIdx = glGetUniformLocation(currProgID, "currNumColliders");
	glUniform1ui(uniformIdx, boxColliders.size());

	subroutineIdx = glGetSubroutineIndex(currProgID, GL_COMPUTE_SHADER, "sweepX");
	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &subroutineIdx);

	glDispatchComputeIndirect(0);
	mAtomicCounter.lock();
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	// SORT ON Y-AXIS
	currProgID = mComputeProgramIDContainer[P3_ODD_EVEN_SORT];
	glUseProgram(currProgID);

	uniformIdx = glGetUniformLocation(currProgID, "currNumObjects");
	CHECKED_GL_CALL(glUniform1ui(uniformIdx, boxColliders.size()));

	uniformIdx = glGetUniformLocation(currProgID, "evenOrOdd");

	subroutineIdx = glGetSubroutineIndex(currProgID, GL_COMPUTE_SHADER, "sortY");
	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &subroutineIdx);

	iterations = boxColliders.size();
	while (iterations--)
	{
		//--------------------------- Dispatch and synchronize ---------------------------//
		// -1 for even, 1 for odd; we start processing the odd pair first
		glUniform1i(uniformIdx, 1);
		glDispatchComputeIndirect(0);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glUniform1i(uniformIdx, -1);
		glDispatchComputeIndirect(0);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	// SWEEP AND PRUNE ON Y-AXIS
	currProgID = mComputeProgramIDContainer[P3_SAP];
	glUseProgram(currProgID);

	mAtomicCounter.reset();

	uniformIdx = glGetUniformLocation(currProgID, "currNumColliders");
	glUniform1ui(uniformIdx, boxColliders.size());

	subroutineIdx = glGetSubroutineIndex(currProgID, GL_COMPUTE_SHADER, "sweepY");
	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &subroutineIdx);

	glDispatchComputeIndirect(0);
	mAtomicCounter.lock();
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	// SORT ON Z-AXIS
	currProgID = mComputeProgramIDContainer[P3_ODD_EVEN_SORT];
	glUseProgram(currProgID);

	uniformIdx = glGetUniformLocation(currProgID, "currNumObjects");
	CHECKED_GL_CALL(glUniform1ui(uniformIdx, boxColliders.size()));

	uniformIdx = glGetUniformLocation(currProgID, "evenOrOdd");

	subroutineIdx = glGetSubroutineIndex(currProgID, GL_COMPUTE_SHADER, "sortZ");
	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &subroutineIdx);

	iterations = boxColliders.size();
	while (iterations--)
	{
		//--------------------------- Dispatch and synchronize ---------------------------//
		// -1 for even, 1 for odd; we start processing the odd pair first
		glUniform1i(uniformIdx, 1);
		glDispatchComputeIndirect(0);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glUniform1i(uniformIdx, -1);
		glDispatchComputeIndirect(0);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	// SWEEP AND PRUNE ON Z-AXIS
	currProgID = mComputeProgramIDContainer[P3_SAP];
	glUseProgram(currProgID);

	mAtomicCounter.reset();

	uniformIdx = glGetUniformLocation(currProgID, "currNumColliders");
	glUniform1ui(uniformIdx, boxColliders.size());

	subroutineIdx = glGetSubroutineIndex(currProgID, GL_COMPUTE_SHADER, "sweepZ");
	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &subroutineIdx);

	glDispatchComputeIndirect(0);
	mAtomicCounter.lock();
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	// Reset and unbind
	mAtomicCounter.reset();
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0u, 0u);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1u, 0u);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0u);
	glUseProgram(0u);
}
