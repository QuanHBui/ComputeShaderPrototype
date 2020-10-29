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

void P3OpenGLComputeBroadPhase::step(std::vector<P3BoxCollider> const &boxColliders)
{
	detectCollisionPairs(boxColliders);
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
	mComputeProgramIDContainer[P3_UPDATE_AABBS] = createComputeProgram("../resources/shaders/updateAABBs.comp");
	mComputeProgramIDContainer[P3_ODD_EVEN_SORT] = createComputeProgram("../resources/shaders/evenOddSort.comp");
	mComputeProgramIDContainer[P3_DETECT_PAIRS] = createComputeProgram("../resources/shaders/detectPairs.comp");
	// mComputeProgramIDContainer[P3_ASSIGN_MORTON_CODES] = createComputeProgram("../resources/shaders/assignMortonCodes.comp");
	// mComputeProgramIDContainer[P3_BUILD_PARALLEL_LINEAR_BVH] = createComputeProgram("../resources/shaders/buildParallelLinearBVH.comp");
	// mComputeProgramIDContainer[P3_SORT_LEAF_NODES] = createComputeProgram("../resources/shaders/sortLeafNodes.comp");
}

void P3OpenGLComputeBroadPhase::initGpuBuffers()
{
	CHECKED_GL_CALL(glGenBuffers(NUM_BROAD_PHASE_SSBOS, mSsboIDContainer.data()));

	glGenBuffers(1, &mAtomicBufferID);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, mAtomicBufferID);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
}

void P3OpenGLComputeBroadPhase::buildBvhTreeOnGpu()
{

}

void P3OpenGLComputeBroadPhase::detectCollisionPairs(std::vector<P3BoxCollider> const &boxColliders)
{
	// Bind mesh collider buffer
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSsboIDContainer[P3_BOX_COLLIDERS]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(P3BoxCollider) * boxColliders.size(),
		boxColliders.data(), GL_STATIC_READ);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSsboIDContainer[P3_AABBS]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(AabbGpuPackage), nullptr, GL_DYNAMIC_COPY);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0u, mSsboIDContainer[P3_BOX_COLLIDERS]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1u, mSsboIDContainer[P3_AABBS]);

	//================== Start of update AABBs ==================//
	GLuint currProgID = mComputeProgramIDContainer[P3_UPDATE_AABBS];
	glUseProgram(currProgID);

	GLuint uniformIdx = glGetUniformLocation(currProgID, "currNumColliders");
	glUniform1ui(uniformIdx, boxColliders.size());

	glDispatchCompute(GLuint(1), GLuint(1), GLuint(1));
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	//----------------- Debug for update AABBs -----------------//
	// glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSsboIDContainer[P3_AABBS]);
	// GLvoid *pGpuMem = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	// memcpy(&mAabbCpuData, pGpuMem, sizeof(AabbGpuPackage));
	// glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	// for (int i = 0; i < 5; ++i)
	// {
	// 	printf("%.03f\t%.03f\n", mAabbCpuData.minCoords[i].x, mAabbCpuData.maxCoords[i].x);
	// 	fflush(stdout);
	// }
	//------------- End debug for Update AABBs -------------//

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
		CHECKED_GL_CALL(glUniform1i(uniformIdx, 1));
		CHECKED_GL_CALL(glDispatchCompute(GLuint(1), GLuint(1), GLuint(1)));
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		CHECKED_GL_CALL(glUniform1i(uniformIdx, -1));
		CHECKED_GL_CALL(glDispatchCompute(GLuint(1), GLuint(1), GLuint(1)));
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	// SWEEP AND PRUNE ON X-AXIS
	currProgID = mComputeProgramIDContainer[P3_DETECT_PAIRS];
	glUseProgram(currProgID);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSsboIDContainer[P3_COLLISION_PAIRS]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 2 * MAX_NUM_COLLIDERS * sizeof(glm::vec4), nullptr, GL_DYNAMIC_COPY);

	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0u, mAtomicBufferID);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2u, mSsboIDContainer[P3_COLLISION_PAIRS]);

	uniformIdx = glGetUniformLocation(currProgID, "currNumColliders");
	glUniform1ui(uniformIdx, boxColliders.size());

	subroutineIdx = glGetSubroutineIndex(currProgID, GL_COMPUTE_SHADER, "sweepX");
	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &subroutineIdx);

	glDispatchCompute(GLuint(1), GLuint(1), GLuint(1));
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT || GL_ATOMIC_COUNTER_BARRIER_BIT);

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
		CHECKED_GL_CALL(glUniform1i(uniformIdx, 1));
		CHECKED_GL_CALL(glDispatchCompute(GLuint(1), GLuint(1), GLuint(1)));
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		CHECKED_GL_CALL(glUniform1i(uniformIdx, -1));
		CHECKED_GL_CALL(glDispatchCompute(GLuint(1), GLuint(1), GLuint(1)));
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	// SWEEP AND PRUNE ON Y-AXIS
	currProgID = mComputeProgramIDContainer[P3_DETECT_PAIRS];
	glUseProgram(currProgID);

	// Reset collision pair buffer
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSsboIDContainer[P3_COLLISION_PAIRS]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(CollisionPairGpuPackage), nullptr, GL_DYNAMIC_COPY);

	int suff = sizeof(CollisionPairGpuPackage);

	// Reset atomic counter
	resetAtomicCounter();

	uniformIdx = glGetUniformLocation(currProgID, "currNumColliders");
	glUniform1ui(uniformIdx, boxColliders.size());

	subroutineIdx = glGetSubroutineIndex(currProgID, GL_COMPUTE_SHADER, "sweepY");
	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &subroutineIdx);

	glDispatchCompute(GLuint(1), GLuint(1), GLuint(1));
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT || GL_ATOMIC_COUNTER_BARRIER_BIT);

	// SORT ON Z-AXIS
	currProgID = mComputeProgramIDContainer[P3_ODD_EVEN_SORT];
	glUseProgram(currProgID);

	uniformIdx = glGetUniformLocation(currProgID, "currNumObjects");
	CHECKED_GL_CALL(glUniform1ui(uniformIdx, boxColliders.size()));

	subroutineIdx = glGetSubroutineIndex(currProgID, GL_COMPUTE_SHADER, "sortZ");
	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &subroutineIdx);

	iterations = boxColliders.size();
	while (iterations--)
	{
		//--------------------------- Dispatch and synchronize ---------------------------//
		// -1 for even, 1 for odd; we start processing the odd pair first
		CHECKED_GL_CALL(glUniform1i(uniformIdx, 1));
		CHECKED_GL_CALL(glDispatchCompute(GLuint(1), GLuint(1), GLuint(1)));
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		CHECKED_GL_CALL(glUniform1i(uniformIdx, -1));
		CHECKED_GL_CALL(glDispatchCompute(GLuint(1), GLuint(1), GLuint(1)));
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	// SWEEP AND PRUNE ON Y-AXIS
	currProgID = mComputeProgramIDContainer[P3_DETECT_PAIRS];
	glUseProgram(currProgID);

	// Reset collision pair buffer
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSsboIDContainer[P3_COLLISION_PAIRS]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 2 * MAX_NUM_COLLIDERS * sizeof(glm::vec4), nullptr, GL_DYNAMIC_COPY);

	// Reset atomic counter
	resetAtomicCounter();

	uniformIdx = glGetUniformLocation(currProgID, "currNumColliders");
	glUniform1ui(uniformIdx, boxColliders.size());

	subroutineIdx = glGetSubroutineIndex(currProgID, GL_COMPUTE_SHADER, "sweepZ");
	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &subroutineIdx);

	glDispatchCompute(GLuint(1), GLuint(1), GLuint(1));
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT || GL_ATOMIC_COUNTER_BARRIER_BIT);

	//----------------- Debug for SAP -----------------//
	GLint n = 0;
	glGetIntegerv(GL_MAX_SUBROUTINES, &n);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSsboIDContainer[P3_COLLISION_PAIRS]);
	void *pGpuMem = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	memcpy(&mCollisionPairCpuData, pGpuMem, sizeof(CollisionPairGpuPackage));
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	for (int i = 0; i < 20; ++i)
	{
		printf("%.03f\t%.03f\n", mCollisionPairCpuData.collisionPairs[i].x, mCollisionPairCpuData.collisionPairs[i].y);
		fflush(stdout);
	}
	printf("\n");
	//-------------- End debug for SAP --------------//

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