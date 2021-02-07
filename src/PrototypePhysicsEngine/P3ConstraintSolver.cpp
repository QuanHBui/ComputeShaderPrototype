#include "P3ConstraintSolver.h"

#include "P3Collider.h"
#include "P3NarrowPhaseCommon.h"
#include "P3Transform.h"

void P3ConstraintSolver::solve(
	ManifoldGpuPackage const &manifoldPkg,
	std::vector<P3BoxCollider> const &boxColliderContainer,
	std::vector<LinearTransform> const &rigidLinearTransformContainer,
	std::vector<AngularTransform> const &rigidAngularTransformContainer )
{
	// Solve floor constraint first
	for (LinearTransform const &linearTransform : rigidLinearTransformContainer)
	{
		glm::vec3 finalLinearImpulse{};

		if (rigidLinearTransformContainer[0].position.y <= -3.0f)
		{
			finalLinearImpulse -= rigidLinearTransformContainer[0].velocity;
		}

		if (!mLinearImpulseContainer.empty())
			mLinearImpulseContainer.back() = finalLinearImpulse;
		else
			mLinearImpulseContainer.emplace_back(finalLinearImpulse);
	}

	// Then solve contact constraints - Iterate through all manifolds
	for (int i = 0; i < manifoldPkg.misc.x; ++i)
	{
		glm::vec3 finalLinearImpulse{};
		glm::vec3 finalAngularImpulse{};

		// A manifold contains collision info of a pair of objects, namely their handles/IDs.
		//  And we can use the IDs to access the component container.
		Manifold const &manifold = manifoldPkg.manifolds[i];
		int referenceBoxIdx = manifold.contactBoxIndicesAndContactCount.x; // We don't know what type the object is based on their IDs
		int incidentBoxIdx  = manifold.contactBoxIndicesAndContactCount.y;

		// Solve constraint in pair, so 2 times every time

		// Static contact constraint - Compute both constraint solving linear and angular impulses
		LinearTransform const &linearTransform = rigidLinearTransformContainer[0];
		AngularTransform const &angularTransform = rigidAngularTransformContainer[0];

		// Iterate through each contact points
		for (int contactPointIdx = 0
			; contactPointIdx < manifold.contactBoxIndicesAndContactCount.z
			; ++contactPointIdx)
		{
			glm::vec3 contactPointPos = manifold.contactPoints[contactPointIdx];

			// Reject contact points that are not a part of the solving body
			glm::vec3 r = contactPointPos - linearTransform.position;
			glm::vec3 boxColliderCornerExtension = boxColliderContainer[incidentBoxIdx].mVertices[0];
			if (glm::length(r) > glm::length(boxColliderCornerExtension - linearTransform.position))
				continue;

			// Linear final impulse
			finalLinearImpulse += glm::dot(glm::normalize(r), glm::vec3(manifold.contactNormal))
								* glm::length(linearTransform.velocity) * glm::vec3(-r);
			//finalLinearImpulse += glm::vec3(0.5f, 0.5f, 0.0f);

			// Angular final impulse
			finalAngularImpulse += 0.01f * glm::cross(r, glm::vec3(manifold.contactNormal));
		}

		// This is freaking stupid.
		mLinearImpulseContainer.back() += finalLinearImpulse;
		// Wow, I'm dumb!
		if (!mAngularImpulseContainer.empty())
			mAngularImpulseContainer.back() = finalLinearImpulse;
		else
			mAngularImpulseContainer.emplace_back(finalLinearImpulse);
	}
}

void P3ConstraintSolver::solve()
{

}