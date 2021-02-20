#pragma once

#ifndef P3_ENGINE
#define P3_ENGINE

#include <array>
#include <cassert>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <glad/glad.h>
#include <glm/gtc/quaternion.hpp>

#ifndef P3_COMMON_H
#define P3_COMMON_H

constexpr int cBoxColliderFaceCount = 6;
constexpr int cBoxColliderVertCount = 8;
constexpr int cMaxContactPointCount = 16;
constexpr int cMaxColliderCount = 1024;
constexpr int cMaxObjectCount = 1024;

#endif // P3_COMMON_H

//===========================================================================//

#ifndef ATOMIC_COUNTER_H
#define ATOMIC_COUNTER_H

class AtomicCounter
{
public:
	void init();

	void bind()
	{
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, mAtomicBufferID);
	}

	void bindTo(GLuint bindIdx)
	{
		glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, bindIdx, mAtomicBufferID);
	}

	void unbind()
	{
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0u);
	}

	// Since this is created with coherent bit, no need to sync
	GLuint get()
	{
		return *mpAtomicCounter;
	}

	void reset()
	{
		*mpAtomicCounter = 0u;
	}

	void clear()
	{
		glUnmapBuffer(mAtomicBufferID);
		glDeleteBuffers(1, &mAtomicBufferID);
	}

private:
	GLuint mAtomicBufferID = 0u;
	GLuint *mpAtomicCounter = nullptr;
};

#endif // ATOMIC_COUNTER

//===========================================================================//

#ifndef BOUNDING_VOLUME_H
#define BOUNDING_VOLUME_H

enum class BoundingType : uint16_t
{
	box = 0,
	sphere,
	capsule,
	convex_hull,
	mesh
};

struct BoundingVolume
{
	BoundingVolume() {}
	BoundingVolume(glm::vec3 position) : mPosition{ position } {}
	virtual ~BoundingVolume() {}

	glm::vec3 mPosition{ 0.f };
};

// This is not axis-aligned.
struct Box : public BoundingVolume
{
	Box() {}
};

struct Sphere : public BoundingVolume
{
	Sphere() {}
	Sphere(float radius) : mRadius{ radius } {}

	float mRadius{ 0.f };
};

struct Capsule : public BoundingVolume
{

};

struct ConvexHull : public BoundingVolume
{

};

struct Mesh : public BoundingVolume
{

};

#endif // BOUNDING_VOLUME_H

//===========================================================================//

#ifndef COMPUTE_PROGRAM_H
#define COMPUTE_PROGRAM_H

struct DispatchIndirectCommand
{
	GLuint numGroupsX;
	GLuint numGroupsY;
	GLuint numGroupsZ;
};

GLuint createComputeProgram(std::string const &);

#endif // COMPUTE_PROGRAM_H

//===========================================================================//

#ifndef OPENGL_UTILS
#define OPENGL_UTILS

namespace oglutils
{
	std::string readFileAsString(const std::string &fileName);
	void getComputeShaderInfo();
	void getUboInfo();

	inline GLsync lock()
	{
		return glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	}

	inline void wait(GLsync syncObj)
	{
		assert(syncObj);

		GLenum waitReturnStatus = GL_UNSIGNALED;

		while (waitReturnStatus != GL_ALREADY_SIGNALED && waitReturnStatus != GL_CONDITION_SATISFIED)
		{
			waitReturnStatus = glClientWaitSync(syncObj, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
		}

		glDeleteSync(syncObj);
	}

	inline void checkGLError()
	{
		assert(glGetError() == GL_NO_ERROR);
	}
}
#endif // OPENGL_UTILS

//===========================================================================//

#ifndef P3_TRANSFORM_H
#define P3_TRANSFORM_H

/**
 * This aims to be used as a component. Should be an upgrade to the other rigid body class
 * Based on Baraff-Witkins lecture notes and Glenn Fiedler's blog post
 * http://www.cs.cmu.edu/~baraff/sigcourse/notesd1.pdf
 * https://gafferongames.com/post/physics_in_3d/
 *
 * @author: Quan Bui
 * @version: 09/28/2020
 */

struct LinearTransform
{
	LinearTransform() {};
	LinearTransform(float, float, glm::vec3 const &, glm::vec3 const &, glm::vec3 const &);

	//----------------- Constant quantities -----------------//
	float mass = 0.0f;
	float inverseMass = 0.0f;

	//----------------- State variables -----------------//
	glm::vec3 position{};
	glm::vec3 velocity{};
	glm::vec3 momentum{};
};

//------------------ Data pack for the GPU (SoA) --------------------//
struct LinearTransformGpuPackage
{
	glm::vec4 positions[cMaxObjectCount]{};
	glm::vec4 velocities[cMaxObjectCount]{};
	glm::vec4 masses[cMaxObjectCount]{};
};

struct AngularTransform
{
	//----------------- Constant quantities -----------------//
	float inertia = 0.0f;
	float inverseInertia = 0.0f;

	//----------------- State variables -----------------//
	float tempOrientation = 0.0f; // Placeholder for rotational angle
	glm::quat orientation{};
	glm::quat spin{};
	glm::vec3 angularVelocity{};
	glm::vec3 angularMomentum{};
};

#endif // P3_TRANSFORM_H

//===========================================================================//

#ifndef P3_COLLIDER_H
#define P3_COLLIDER_H

constexpr glm::vec4 cInstanceVertices[cBoxColliderVertCount] =
{
	glm::vec4{ -1.0f,  1.0f,  1.0f,  1.0f },
	glm::vec4{  1.0f,  1.0f,  1.0f,  1.0f },
	glm::vec4{  1.0f, -1.0f,  1.0f,  1.0f },
	glm::vec4{ -1.0f, -1.0f,  1.0f,  1.0f },

	glm::vec4{ -1.0f,  1.0f, -1.0f,  1.0f },
	glm::vec4{  1.0f,  1.0f, -1.0f,  1.0f },
	glm::vec4{  1.0f, -1.0f, -1.0f,  1.0f },
	glm::vec4{ -1.0f, -1.0f, -1.0f,  1.0f }
};

class P3Collider
{
public:
	virtual glm::vec3 findFarthestPoint(glm::vec3 const &) const = 0;
};

class P3MeshCollider : public P3Collider
{
public:
	void update(glm::mat4 const &model)
	{
		for (unsigned int i = 0u; i < mVertices.size(); ++i)
		{
			mVertices[i] = model * mInstanceVertices[i];
			mVertices[i].w = 1.0f;
		}
	}

	void setInstanceVertices(std::vector<glm::vec4> const &vertices)
	{
		mInstanceVertices = vertices;
	}

	glm::vec3 findFarthestPoint(glm::vec3 const &) const override;

private:
	std::vector<glm::vec4> mVertices =
	{
		glm::vec4{ -1.0f,  1.0f,  1.0f,  1.0f },
		glm::vec4{  1.0f,  1.0f,  1.0f,  1.0f },
		glm::vec4{  1.0f, -1.0f,  1.0f,  1.0f },
		glm::vec4{ -1.0f, -1.0f,  1.0f,  1.0f },

		glm::vec4{ -1.0f,  1.0f, -1.0f,  1.0f },
		glm::vec4{  1.0f,  1.0f, -1.0f,  1.0f },
		glm::vec4{  1.0f, -1.0f, -1.0f,  1.0f },
		glm::vec4{ -1.0f, -1.0f, -1.0f,  1.0f }
	}; // A unit box

	std::vector<glm::vec4> mInstanceVertices =
	{
		glm::vec4{ -1.0f,  1.0f,  1.0f,  1.0f },
		glm::vec4{  1.0f,  1.0f,  1.0f,  1.0f },
		glm::vec4{  1.0f, -1.0f,  1.0f,  1.0f },
		glm::vec4{ -1.0f, -1.0f,  1.0f,  1.0f },

		glm::vec4{ -1.0f,  1.0f, -1.0f,  1.0f },
		glm::vec4{  1.0f,  1.0f, -1.0f,  1.0f },
		glm::vec4{  1.0f, -1.0f, -1.0f,  1.0f },
		glm::vec4{ -1.0f, -1.0f, -1.0f,  1.0f }
	};
};

// A collection of 8 vec4's
struct P3BoxCollider
{
	void update(glm::mat4 const &model)
	{
		for (int i = 0; i < cBoxColliderVertCount; ++i)
		{
			mVertices[i] = model * mInstanceVertices[i];
			mVertices[i].w = 1.0f;
		}
	}

	void setInstanceVertices(glm::vec4 *vertices)
	{
		for (int i = 0; i < cBoxColliderVertCount; ++i)
		{
			mInstanceVertices[i] = vertices[i];
		}
	}

	glm::vec4 mVertices[cBoxColliderVertCount] =
	{
		glm::vec4{ -1.0f,  1.0f,  1.0f,  1.0f }, // 0
		glm::vec4{  1.0f,  1.0f,  1.0f,  1.0f }, // 1
		glm::vec4{  1.0f, -1.0f,  1.0f,  1.0f }, // 2
		glm::vec4{ -1.0f, -1.0f,  1.0f,  1.0f }, // 3

		glm::vec4{ -1.0f,  1.0f, -1.0f,  1.0f }, // 4
		glm::vec4{  1.0f,  1.0f, -1.0f,  1.0f }, // 5
		glm::vec4{  1.0f, -1.0f, -1.0f,  1.0f }, // 6
		glm::vec4{ -1.0f, -1.0f, -1.0f,  1.0f }  // 7
	};

	glm::vec4 mInstanceVertices[cBoxColliderVertCount] =
	{
		glm::vec4{ -1.0f,  1.0f,  1.0f,  1.0f },
		glm::vec4{  1.0f,  1.0f,  1.0f,  1.0f },
		glm::vec4{  1.0f, -1.0f,  1.0f,  1.0f },
		glm::vec4{ -1.0f, -1.0f,  1.0f,  1.0f },

		glm::vec4{ -1.0f,  1.0f, -1.0f,  1.0f },
		glm::vec4{  1.0f,  1.0f, -1.0f,  1.0f },
		glm::vec4{  1.0f, -1.0f, -1.0f,  1.0f },
		glm::vec4{ -1.0f, -1.0f, -1.0f,  1.0f }
	};
};

#endif // P3_COLLIDER_H

//===========================================================================//

#ifndef P3_BROAD_PHASE_COMMON
#define P3_BROAD_PHASE_COMMON

//------------------ Data packs for the GPU (SoA) --------------------//
struct AabbGpuPackage
{
	glm::vec4 minCoords[cMaxObjectCount]{};
	glm::vec4 maxCoords[cMaxObjectCount]{};
};

// This potentially can store the size of the boxColliders, to solve the potential
//  padding issue, the size can be padded to 16 bytes, or just use a glm::vec4
struct BoxColliderGpuPackage
{
	glm::vec4 const *operator[](int boxIdx) const { return boxColliders[boxIdx]; }

	glm::ivec4 misc{};
	glm::vec4 boxColliders[cMaxObjectCount][cBoxColliderVertCount]{};
};

struct CollisionPairGpuPackage
{
	glm::vec4 const &operator[](int boxIdx) const { return collisionPairs[boxIdx]; }

	glm::ivec4 misc{};
	glm::ivec4 collisionPairs[2 * cMaxObjectCount]{};
};

#endif // P3_BROAD_PHASE_COMMON

//===========================================================================//

#ifndef P3_OPENGL_COMPUTE_BROAD_PHASE_H
#define P3_OPENGL_COMPUTE_BROAD_PHASE_H

constexpr int cBroadPhaseComputeProgramCount = 5;
constexpr GLsizei cBroadPhaseSsboCount = 3;

class P3OpenGLComputeBroadPhase
{
public:
	void init();

	//--------------------- Main proccesses --------------------------//
	void step(std::vector<P3BoxCollider> const &);
	//----------------------------------------------------------------//

	GLuint getBoxCollidersID() const { return mSsboIDs[P3_BOX_COLLIDERS]; };
	GLuint getCollisionPairsID() const { return mSsboIDs[P3_COLLISION_PAIRS]; }

	CollisionPairGpuPackage const *getPCollisionPairPkg() const { return mpCollisionPairPkg; }

	void reset();

	// TODO: Deallocate all Gpu buffers and delete programs.
	~P3OpenGLComputeBroadPhase() {}

private:
	void initShaderPrograms();
	GLuint initGpuBuffers();

	void buildBvhTreeOnGpu();
	void detectCollisionPairs(std::vector<P3BoxCollider> const &);

	void resetAtomicCounter();
	GLuint readAtomicCounter();

	// Indicies of shader programs
	enum
	{
		P3_UPDATE_AABBS = 0,
		P3_ODD_EVEN_SORT,
		P3_SAP,
		P3_ASSIGN_MORTON_CODES,
		P3_SORT_LEAF_NODES,
		P3_BUILD_PARALLEL_LINEAR_BVH
	};

	// Indicices of ssbo's
	enum
	{
		P3_BOX_COLLIDERS = 0,
		P3_AABBS,
		P3_COLLISION_PAIRS
	};

	//----------------------------- OpenGL bookkeeping ----------------------------//
	std::array<GLuint, cBroadPhaseComputeProgramCount> mComputeProgramIDContainer{};
	std::array<GLuint, cBroadPhaseSsboCount> mSsboIDs{};

	GLuint mDispatchIndirectBufferID = 0;
	DispatchIndirectCommand mDispatchIndirectCommand{ 1, 1, 1 };

	//--------------------------------- CPU data ---------------------------------//
	AtomicCounter mAtomicCounter[3]; // Triple buffering let's go
	AabbGpuPackage mAabbCpuData;
	BoxColliderGpuPackage *mpBoxColliderPkg = nullptr; // Data streaming to GPU
	CollisionPairGpuPackage *mpCollisionPairPkg = nullptr;
};

#endif // P3_OPENGL_COMPUTE_BROAD_PHASE_H

//===========================================================================//

#ifndef P3_NARROW_PHASE_COMMON_H
#define P3_NARROW_PHASE_COMMON_H

struct Manifold
{
	glm::ivec4 contactBoxIndicesAndContactCount{}; // x = refBoxIdx, y = incidentBoxIdx, z = contact count
	glm::vec4 contactPoints[cMaxContactPointCount]{};
	glm::vec4 contactNormal{}; // w stores the penetration depth.
};

struct ManifoldGpuPackage // To be replaced by the struct below
{
	glm::ivec4 misc{};
	Manifold manifolds[cMaxColliderCount]{};
};

// Let's make this more data driven, meaning it doesn't make any physical sense but it's easy to move/map data around.
struct ManifoldPackage
{
	glm::ivec4 misc{}; // Must use ivec4 for appropriate memory padding, real data start at offset of 16 bytes
	glm::ivec4 contactBoxIndicesAndContactCount[cMaxColliderCount]{};
	glm::vec4 contactPoints[cMaxContactPointCount][cMaxColliderCount]{};
	glm::vec4 contactNormal[cMaxColliderCount]{};
};

#endif // P3_NARROW_PHASE_COMMON_H

//===========================================================================//

#ifndef P3_SAT_H
#define P3_SAT_H

/**
 * This is the implementation of Seperating Axis Test on the CPU.
 */
ManifoldGpuPackage P3Sat(BoxColliderGpuPackage const &, const CollisionPairGpuPackage *);

#endif // P3_SAT_H

//===========================================================================//

#ifndef P3_OPENGL_COMPUTE_NARROW_PHASE_H
#define P3_OPENGL_COMPUTE_NARROW_PHASE_H

constexpr uint16_t cNarrowPhaseComputeProgramCount = 1u;
constexpr GLsizei cNarrowPhaseSsboCount = 1u;


class P3OpenGLComputeNarrowPhase
{
public:
	void init(GLuint, GLuint);

	void step();

	const ManifoldGpuPackage *getPManifoldPkg() const { return mpManifoldPkg; }

	void reset();

	~P3OpenGLComputeNarrowPhase()
	{
		mAtomicCounter.clear();
	}

private:
	void initShaderPrograms()
	{
		mComputeProgIDs[ComputeShader::SAT] = createComputeProgram("../resources/shaders/sat.comp");
	}

	void initGpuBuffers();

	enum class ComputeShader
	{
		SAT,
		TRI_TRI_TEST
	};

	enum class Buffer
	{
		EXPERIMENTAL,
		BOX_COLLIDER,
		COLLISION_PAIR,
		MANIFOLD
	};

	GLuint mComputeProgramIDs[cNarrowPhaseComputeProgramCount]{};
	std::unordered_map<ComputeShader, GLuint> mComputeProgIDs{};
	std::unordered_map<Buffer, GLuint> mSsboIDs{};

	AtomicCounter mAtomicCounter{};
	ManifoldGpuPackage *mpManifoldPkg = nullptr; // Stores the results from last physics tick
};

#endif // P3_OPENGL_COMPUTE_NARROW_PHASE_H

//===========================================================================//

#ifndef P3_CONSTRAINT_SOLVER
#define P3_CONSTRAINT_SOLVER

struct ManifoldGpuPackage;

/**
 * Take in a box collider list and a contact manifold list, then return an
 *  impulse list for resolution
 *
 * Much like the collision detection phase, this is also parallelized on the GPU
 *
 * Now, how do I make this configurable? Or what is there to configure?
 */
class P3ConstraintSolver
{
public:
	P3ConstraintSolver() {}

	// TODO: Overloaded constructor for any configuration setting
	P3ConstraintSolver(GLuint boxCollidersID, GLuint manifoldsID)
		: mBoxCollidersID(boxCollidersID), mManifoldsID(manifoldsID) {}

	// By default, reserve enough space for 10 things.
	void init(int reserveObjCount = 10)
	{
		for (int i = 0; i < reserveObjCount; ++i)
		{
			mLinearImpulseContainer.emplace_back();
			mAngularImpulseContainer.emplace_back();
		}
	};

	// If the solver's implementation resides on the CPU, returns the offset
	void solve(ManifoldGpuPackage const &,
		std::vector<P3BoxCollider> const &,
		std::vector<LinearTransform> const &,
		std::vector<AngularTransform> const &);
	// Else if on GPU, prob needs to know the handles of ManifoldGpuPackage from init()
	void solve();

	std::vector<glm::vec3> const &getLinearImpulseContainer() const { return mLinearImpulseContainer; }
	std::vector<glm::vec3> const &getAngularImpulseContainer() const { return mAngularImpulseContainer; }

	void reset();

	~P3ConstraintSolver() {}

private:
	GLuint mBoxCollidersID = 0u, mManifoldsID = 0u;

	std::vector<glm::vec3> mLinearImpulseContainer;
	std::vector<glm::vec3> mAngularImpulseContainer;

	int mResetCounter = 0;
};

#endif // P3_CONSTRAINT_SOLVER

//===========================================================================//

#ifndef P3_INTEGRATOR
#define P3_INTEGRATOR

class P3Integrator
{

};

#endif // P3_INTEGRATOR

//===========================================================================//

#ifndef P3_DYNAMICS_WORLD_H
#define P3_DYNAMICS_WORLD_H

#define NARROW_PHASE_CPU

using LinearTransformContainerPtr = std::shared_ptr<std::vector<LinearTransform>>;

class P3DynamicsWorld
{
public:
	P3DynamicsWorld() {}
	P3DynamicsWorld(size_t maxCapacity) : mMaxCapacity(maxCapacity) {} // Might want error checking here

	void init();

	void detectCollisions();

	void updateMultipleBoxes(float dt);
	void updateBowlingGame(float dt);
	void updateControllableBox(float dt, glm::vec3 const &);
	void updateGravityTest(float dt);

	glm::vec3 castRay(glm::vec3 const &, glm::vec3 const &);

	//---------------------- Add bodies to the world ----------------------//
	// Is it the world responsibility to check for max capacity before adding?
	int addRigidBody();
	int addRigidBody(float, glm::vec3 const &, glm::vec3 const &);
	int addRigidBody(float, glm::vec3 const &, glm::vec3 const &, glm::mat4 const &);
	int addRigidBody(LinearTransform const &, AngularTransform const &);

	int addStaticBody(glm::vec3 const &);
	int addStaticBodies(std::vector<glm::vec3> const &);

	//------------------------ Demos ------------------------//
	void reset();
	void fillWorldWithBodies();
	void bowlingGameDemo();
	void stackingSpheresDemo();
	void stackingBoxesDemo();
	void multipleBoxesDemo();
	void controllableBoxDemo();

	//----------------------- Some getters and setters -----------------------//
	float getGravity() const { return mGravity; }
	float getAirDrag() const { return mAirDrag; }
	unsigned int getOccupancy() const { return mBodyContainer.size(); }
	unsigned int getNumBoxColliders() const { return mBoxColliderContainer.size(); }
	unsigned int getMaxCapacity() const { return mMaxCapacity; }
	std::vector<P3BoxCollider> const &getBoxColliders() const { return mBoxColliderContainer; }

	std::vector<LinearTransform> const &getRigidLinearTransformContainer() const
	{
		return mRigidLinearTransformContainer;
	}

	std::vector<LinearTransform> const &getStaticLinearTransformContainer() const
	{
		return mStaticLinearTransformContainer;
	}

	std::vector<AngularTransform> const &getRigidAngularTransformContainer() const
	{
		return mRigidAngularTransformContainer;
	}

	std::vector<AngularTransform> const &getStaticAngularTransformContainer() const
	{
		return mStaticAngularTransformContainer;
	}

	CollisionPairGpuPackage const *getPCollisionPairPkg() const
	{
		return mBroadPhase.getPCollisionPairPkg();
	}

	ManifoldGpuPackage const *getPManifoldPkg() const
	{
#ifdef NARROW_PHASE_CPU
		return &mManifoldPkg;
#else
		return mNarrowPhase.getPManifoldPkg();
#endif
	}

	void setGravity(float gravity) { mGravity = gravity; }
	void setMaxCapacity(const int maxCapacity) { mMaxCapacity = maxCapacity; } // Need error checking

	bool isFull() { return mBodyContainer.size() >= mMaxCapacity; }

private:
	//---------------- Constant physics quantities ----------------//
	float mGravity{ 0.001f }, mAirDrag{ 2.0f };
	size_t mMaxCapacity{ 20u };

	//------------------------- Entity list -------------------------//
	std::vector<int> mBodyContainer;
	int mUniqueID = 0u;

	//----------------------- Component list -----------------------//
	std::vector<LinearTransform> mRigidLinearTransformContainer;
	std::vector<AngularTransform> mRigidAngularTransformContainer;
	std::vector<LinearTransform> mStaticLinearTransformContainer;
	std::vector<AngularTransform> mStaticAngularTransformContainer;
	std::vector<P3MeshCollider> mMeshColliderContainer;
	std::vector<P3BoxCollider> mBoxColliderContainer;
	std::vector<glm::mat4> mBoxColliderCtmContainer;

	//----------------- Data package optimized for the GPU -----------------//
	LinearTransformGpuPackage mLinearTransformPkg; // For rigid and kinematic bodies
	ManifoldGpuPackage mManifoldPkg;

	//----------------- Map of index to rigid body -----------------//
	// @reference: https://austinmorlan.com/posts/entity_component_system/
	//std::unordered_map<int, int> mEntityToIndexMap;
	//std::unordered_map<int, int> mIndexToEntityMap;

	//--------------------- Physics pipeline ---------------------//
	// Order of operations for each timestep: Collision -> apply forces -> solve constraints -> update positions
	P3OpenGLComputeBroadPhase mBroadPhase;
	P3OpenGLComputeNarrowPhase mNarrowPhase;
	P3ConstraintSolver mConstraintSolver; // Produces forces to make sure things don't phase past each other
	P3Integrator mIntegrator;             // Actually integrates the force vector and apply to linear transform
};

#endif // P3_DYNAMICS_WORLD_H

#endif // P3_ENGINE_H
