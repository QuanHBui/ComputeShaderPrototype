#include "P3BroadPhaseCollisionDetection.h"

#include "ComputeProgram.h"
#include "GLSL.h"

#ifndef MAX_NUM_COLLIDERS
#define MAX_NUM_COLLIDERS 1000u
#endif

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
	resetAtomicCounter();
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
	glGenBuffers(NUM_BROAD_PHASE_SSBOS, mSsboIDs.data());

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
		mapFlags));

	glGenBuffers(1, &mAtomicBufferID);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, mAtomicBufferID);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

	glGenBuffers(1, &mDispatchIndirectBufferID);
	glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, mDispatchIndirectBufferID);
	glBufferStorage(GL_DISPATCH_INDIRECT_BUFFER, sizeof(DispatchIndirectCommand), &mDispatchIndirectCommand,
		mapFlags);

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

	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0u, mAtomicBufferID);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2u, mSsboIDs[P3_COLLISION_PAIRS]);

	uniformIdx = glGetUniformLocation(currProgID, "currNumColliders");
	glUniform1ui(uniformIdx, boxColliders.size());

	subroutineIdx = glGetSubroutineIndex(currProgID, GL_COMPUTE_SHADER, "sweepX");
	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &subroutineIdx);

	glDispatchComputeIndirect(0);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);

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

	resetAtomicCounter();

	uniformIdx = glGetUniformLocation(currProgID, "currNumColliders");
	glUniform1ui(uniformIdx, boxColliders.size());

	subroutineIdx = glGetSubroutineIndex(currProgID, GL_COMPUTE_SHADER, "sweepY");
	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &subroutineIdx);

	glDispatchComputeIndirect(0);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);

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

	resetAtomicCounter();

	uniformIdx = glGetUniformLocation(currProgID, "currNumColliders");
	glUniform1ui(uniformIdx, boxColliders.size());

	subroutineIdx = glGetSubroutineIndex(currProgID, GL_COMPUTE_SHADER, "sweepZ");
	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &subroutineIdx);

	glDispatchComputeIndirect(0);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);

	// Reset and unbind
	resetAtomicCounter();
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0u, 0u);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0u, 0u);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1u, 0u);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0u);
	glUseProgram(0u);
}

void P3OpenGLComputeBroadPhase::resetAtomicCounter()
{
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, mAtomicBufferID);
	GLuint *pMappedBufferMemory = (GLuint *)glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint),
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
	memset(pMappedBufferMemory, 0, sizeof(GLuint));
	glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
}

GLuint P3OpenGLComputeBroadPhase::readAtomicCounter()
{
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, mAtomicBufferID);
	GLuint *userCounter = (GLuint *)glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER,
		0,
		sizeof(GLuint),
		GL_MAP_READ_BIT
	);
	GLuint theActualCounter = *userCounter;
	glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);

	return theActualCounter;
}
