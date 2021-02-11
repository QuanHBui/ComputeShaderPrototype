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
	// Reset the containers
	if (!mLinearImpulseContainer.empty())
	{
		mLinearImpulseContainer.pop_back();
	}
	if (!mAngularImpulseContainer.empty())
	{
		mAngularImpulseContainer.pop_back();
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

		glm::vec3 prevR{};

		// Iterate through each contact points
		for (int contactPointIdx = 0
			; contactPointIdx < manifold.contactBoxIndicesAndContactCount.z
			; ++contactPointIdx)
		{
			glm::vec3 contactPointPos = manifold.contactPoints[contactPointIdx];

			// Reject contact points that are not a part of the solving body
			glm::vec3 r = contactPointPos - linearTransform.position;
			if (r == prevR) continue;

			glm::vec3 boxColliderCornerExtension = boxColliderContainer[incidentBoxIdx].mVertices[0];

			r = glm::normalize(r);
			// Apply linear impulse onto each contact point
			finalLinearImpulse += 0.5f * (0.5f * glm::dot(-r, glm::vec3(manifold.contactNormal) * -manifold.contactNormal.w)
								+ 0.5f * glm::length(linearTransform.velocity)) * -r;

			// Angular final impulse
			finalAngularImpulse += 0.5f * glm::cross(r, glm::vec3(manifold.contactNormal));

			prevR = r;
		}

		// This is freaking stupid.
		if (!mLinearImpulseContainer.empty())
			mLinearImpulseContainer.back() += finalLinearImpulse;
		else
			mLinearImpulseContainer.emplace_back(finalLinearImpulse);

		// Wow, I'm dumb!
		if (!mAngularImpulseContainer.empty())
			mAngularImpulseContainer.back() = finalAngularImpulse;
		else
			mAngularImpulseContainer.emplace_back(finalAngularImpulse);
	}
}

void P3ConstraintSolver::solve()
{

}