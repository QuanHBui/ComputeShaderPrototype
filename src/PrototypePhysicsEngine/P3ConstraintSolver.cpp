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
	// Prototype temporal coherence, keep the old values for 4 physics ticks, depending on how fast the objects are moving.
	++mResetCounter;
	if (mResetCounter >= 4)
	{
		assert(rigidLinearTransformContainer.size() == rigidAngularTransformContainer.size()); // This should always be the case.

		// Might want to make sure the size of the rigidLinearTransform and mLinearImpulseContainer are roughly the same size.
		for (int i = 0; i < rigidLinearTransformContainer.size(); ++i)
		{
			mLinearImpulseContainer[i] = glm::vec3(0.0f);
		}
		for (int j = 0; j < rigidAngularTransformContainer.size(); ++j)
		{
			mAngularImpulseContainer[j] = glm::vec3(0.0f);
		}

		mResetCounter = 0;
	}

	// Then solve contact constraints - Iterate through all manifolds
	for (int i = 0; mResetCounter == 0 && i < manifoldPkg.misc.x; ++i)
	{
		glm::vec3 finalLinearImpulse{};
		glm::vec3 finalAngularImpulse{};

		// A manifold contains collision info of a pair of objects, namely their handles/IDs.
		//  And we can use the IDs to access the component container.
		Manifold const &manifold = manifoldPkg.manifolds[i];
		int referenceBoxIdx = manifold.contactBoxIndicesAndContactCount.x;
		int incidentBoxIdx  = manifold.contactBoxIndicesAndContactCount.y; // We don't know what type the object is based on their IDs

		// Solve constraint in pair, so 2 times every time, but for now the solver only works on the incident body.

		// Between 2 indices, the smaller index is of rigid body, if the incident index is greater than reference index,
		//  we don't solve that collision because we don't solve reference collision. This is not always the case of course
		if (incidentBoxIdx > referenceBoxIdx) continue;

		// Static contact constraint - Compute both constraint solving linear and angular impulses
		LinearTransform const &linearTransform   = rigidLinearTransformContainer[incidentBoxIdx];
		AngularTransform const &angularTransform = rigidAngularTransformContainer[incidentBoxIdx];

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
			finalLinearImpulse += 0.25f * (0.5f * glm::dot(-r, glm::vec3(manifold.contactNormal))
								+ 0.35f * glm::length(linearTransform.velocity)) * -r;

			// Angular final impulse
			finalAngularImpulse += 0.5f * glm::cross(r, glm::vec3(manifold.contactNormal));

			prevR = r;
		}

		mLinearImpulseContainer[incidentBoxIdx]  += finalLinearImpulse;
		mAngularImpulseContainer[incidentBoxIdx] += finalAngularImpulse;
	}
}

void P3ConstraintSolver::solve()
{

}