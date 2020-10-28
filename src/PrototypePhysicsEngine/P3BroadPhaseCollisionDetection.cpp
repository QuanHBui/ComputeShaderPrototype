#include "P3BroadPhaseCollisionDetection.h"

#include "ComputeProgram.h"
#include "GLSL.h"

void P3CpuBroadPhase(Aabb *pAabbContainer, unsigned int size)
{
	for (unsigned int i = 0u; i < size; ++i)
	{
		Aabb bodyi = pAabbContainer[i];

		for (unsigned int j = i; j < size; ++j)
		{
			Aabb bodyj = pAabbContainer[j];

			// Check if the body is a static body, the one with infinite mass, or inverse mass of 0

			// Create Arbiter

			// Look out for contact points
		}
	}
}

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
	mComputeProgramIDContainer[P3_DETECT_PAIRS] = createComputeProgram("../resources/shaders/detectPairs.comp");
	mComputeProgramIDContainer[P3_ODD_EVEN_SORT] = createComputeProgram("../resources/shaders/evenOddSort.comp");
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
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSsboIDContainer[P3_AABBS]);
	GLvoid *pGpuMem = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	memcpy(&mAabbCpuData, pGpuMem, sizeof(AabbGpuPackage));
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	for (int i = 0; i < 5; ++i)
	{
		printf("%.03f\t%.03f\n", mAabbCpuData.minCoords[i].x, mAabbCpuData.maxCoords[i].x);
		fflush(stdout);
	}
	//------------- End debug for Update AABBs -------------//

	//================ End of update AABBs ================//

	//================== Start of SAP ==================//

	// Sweep on x-axis first
	currProgID = mComputeProgramIDContainer[P3_ODD_EVEN_SORT];
	glUseProgram(currProgID);

	glUniform1ui(uniformIdx, boxColliders.size());
	uniformIdx = glGetUniformLocation(currProgID, "evenOrOdd");

	// Sort on x first
	GLuint subroutineIdx = glGetSubroutineIndex(currProgID, GL_COMPUTE_SHADER, "sweepX");
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

	// Prune on x-axis
	currProgID = mComputeProgramIDContainer[P3_DETECT_PAIRS];
	glUseProgram(currProgID);

	//----------------- Debug for SAP -----------------//
	// GLint n = 0;
	// glGetIntegerv(GL_MAX_SUBROUTINES, &n);

	// glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSsboIDContainer[P3_AABBS]);
	// pGpuMem = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	// memcpy(&mAabbCpuData, pGpuMem, sizeof(AabbGpuPackage));
	// glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	// for (int i = 0; i < 5; ++i)
	// {
	// 	printf("%.03f\t%.03f\n", mAabbCpuData.minCoords[i].x, mAabbCpuData.maxCoords[i].x);
	// 	fflush(stdout);
	// }
	//-------------- End debug for SAP --------------//

	// Unbind
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0u, 0u);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1u, 0u);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0u);
	glUseProgram(0u);

	// GLuint uniformIndex = glGetUniformBlockIndex(mComputeProgramIDContainer[P3_EVEN_ODD_SORT], "evenOrOdd");
	// glUniform1ui(uniformIndex, meshColliderContainer.size());

	// glUseProgram(mComputeProgramIDContainer[P3_DETECT_PAIRS]);

	// glDispatchCompute(1u, 1u, 1u);
	// glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);
}

void P3OpenGLComputeBroadPhase::resetAtomicCounter()
{
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, mAtomicBufferID);
	GLuint *pMappedBufferMemory = (GLuint *)glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint),
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
	memset(pMappedBufferMemory, 0, sizeof(GLuint));
	glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
}