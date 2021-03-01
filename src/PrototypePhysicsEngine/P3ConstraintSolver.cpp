#include "P3ConstraintSolver.h"

#include "P3Collider.h"
#include "P3NarrowPhaseCommon.h"
#include "P3Transform.h"

void P3ConstraintSolver::solve(
	ManifoldGpuPackage const &manifoldPkg,
	std::vector<P3BoxCollider> const &boxColliderContainer,
	std::vector<LinearTransform> const &rigidLinearTransformContainer,
	std::vector<AngularTransform> const &rigidAngularTransformContainer,
	std::vector<LinearTransform> const &staticLinearTransformContainer,
	std::vector<AngularTransform> const &staticAngularTransformContainer )
{
	// Prototype temporal coherence, keep the old values for 4 physics ticks, depending on how fast the objects are moving.
	++mResetCounter;
	if (mResetCounter >= 2)
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

		int referenceTransformContainerIdx = referenceBoxIdx;
		int incidentTransformContainerIdx  = incidentBoxIdx;

		// Solve constraint in pair, so 2 times every time if the 2 bodies are both rigid. This solver doesn't work on static bodies. However,
		//  it does use the info from the static bodies.

		// Forgive me for I have sinned
		const LinearTransform *pReferenceLinearTransform   = nullptr;
		const AngularTransform *pReferenceAngularTransform = nullptr;

		if (referenceBoxIdx >= rigidLinearTransformContainer.size())
		{
			pReferenceLinearTransform  = &staticLinearTransformContainer[referenceBoxIdx - rigidLinearTransformContainer.size()];
			pReferenceAngularTransform = &staticAngularTransformContainer[referenceBoxIdx - rigidAngularTransformContainer.size()];
		}
		else
		{
			pReferenceLinearTransform  = &rigidLinearTransformContainer[referenceBoxIdx];
			pReferenceAngularTransform = &rigidAngularTransformContainer[referenceBoxIdx];
		}

		const LinearTransform *pIncidentLinearTransform   = nullptr;
		const AngularTransform *pIncidentAngularTransform = nullptr;

		if (incidentBoxIdx >= rigidLinearTransformContainer.size())
		{
			pIncidentLinearTransform  = &staticLinearTransformContainer[incidentBoxIdx - rigidLinearTransformContainer.size()];
			pIncidentAngularTransform = &staticAngularTransformContainer[incidentBoxIdx - rigidLinearTransformContainer.size()];
		}
		else
		{
			pIncidentLinearTransform  = &rigidLinearTransformContainer[incidentBoxIdx];
			pIncidentAngularTransform = &rigidAngularTransformContainer[incidentBoxIdx];
		}

		// Iterate through each contact points
		for ( int contactPointIdx = 0
			; contactPointIdx < manifold.contactBoxIndicesAndContactCount.z
			; ++contactPointIdx )
		{
			glm::vec3 contactPointPos = manifold.contactPoints[contactPointIdx];

			// Relative positions of the contact point to the 2 bodies
			glm::vec3 r_1 = contactPointPos - pReferenceLinearTransform->position;
			glm::vec3 r_2 = contactPointPos - pIncidentLinearTransform->position;

			// Relative velocity at contact point
			glm::vec3 dv = pIncidentLinearTransform->velocity + glm::cross(pIncidentAngularTransform->angularVelocity, r_2)
						 - pReferenceLinearTransform->velocity + glm::cross(pReferenceAngularTransform->angularVelocity, r_1);

			// Find the tangent axis to the surface of the reference box - tangent component to simulate friction
			glm::vec3 tangent = glm::normalize(glm::cross(glm::vec3(manifold.contactNormal), pIncidentAngularTransform->angularVelocity));

			// Tangent component of delta v
			float vt = glm::dot(dv, tangent);

			// TODO: To be continue...

			glm::vec3 r = contactPointPos - pIncidentLinearTransform->position;

			glm::vec3 boxColliderCornerExtension = boxColliderContainer[incidentBoxIdx].mVertices[0];

			r = glm::normalize(r);
			// Apply linear impulse onto each contact point
			finalLinearImpulse += 0.20f * (0.3f * glm::dot(-r, glm::vec3(manifold.contactNormal))
								+ 0.35f * (glm::length(pIncidentLinearTransform->velocity) + 0.15f)) * -r;

			// Angular final impulse
			finalAngularImpulse += 0.4f * glm::cross(r, glm::vec3(manifold.contactNormal));
		}

		if (incidentBoxIdx < rigidLinearTransformContainer.size())
		{
			mLinearImpulseContainer[incidentBoxIdx]  += finalLinearImpulse;
			mAngularImpulseContainer[incidentBoxIdx] += finalAngularImpulse;
		}

		// If the reference box is also a rigid body
		if (referenceBoxIdx < rigidLinearTransformContainer.size())
		{
			mLinearImpulseContainer[referenceBoxIdx]  -= finalLinearImpulse;
			mAngularImpulseContainer[referenceBoxIdx] -= finalAngularImpulse;
		}
	}
}

void P3ConstraintSolver::solve()
{

}

void P3ConstraintSolver::preStep( LinearTransform *pReferenceLinearTransform, AngularTransform *pReferenceAngularTransform,
								  LinearTransform *pIncidentLinearTransform, AngularTransform *pIncidentAngularTransform )
{

}