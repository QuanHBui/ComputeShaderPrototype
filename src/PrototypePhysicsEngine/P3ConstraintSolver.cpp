#include "P3ConstraintSolver.h"

#include "P3Collider.h"
#include "P3NarrowPhaseCommon.h"
#include "P3Transform.h"

constexpr float cBaumgarteFactor = 0.001f;
constexpr float cPenetrationSlop = 0.005f;
constexpr int cIterationCount = 100;

// http://box2d.org/2014/02/computing-a-basis/
void computeBasis(const glm::vec4 &a, glm::vec4 &b, glm::vec4 &c)
{
	// Suppose vector a has all equal components and is a unit vector:
	// a = (s, s, s)
	// Then 3*s*s = 1, s = sqrt(1/3) = 0.57735. This means that at
	// least one component of a unit vector must be greater or equal
	// to 0.57735.

	glm::vec3 aVec3 = a;
	glm::vec3 bVec3 = b;
	glm::vec3 cVec3 = c;

	if (glm::abs(a.x) >= 0.57735f)
		bVec3 = glm::vec3(a.y, -a.x, 0.0f);
	else
		bVec3 = glm::vec3(0.0f, a.z, -a.y);

	bVec3 = glm::normalize(bVec3);
	cVec3 = glm::cross(aVec3, bVec3);

	b = glm::vec4(bVec3, 0.0f);
	c = glm::vec4(cVec3, 0.0f);
}

// Heavily inspired by qu3e physics engine by Randy Gaul
void P3ConstraintSolver::preSolve( ManifoldGpuPackage &manifoldPkg,
								   std::vector<LinearTransform> &rigidLinearTransformContainer,
								   std::vector<AngularTransform> &rigidAngularTransformContainer,
								   std::vector<LinearTransform> &staticLinearTransformContainer,
								   std::vector<AngularTransform> &staticAngularTransformContainer,
								   float dt )
{
	// Then solve contact constraints - Iterate through all manifolds
	for (int i = 0; i < manifoldPkg.misc.x; ++i)
	{
		glm::vec3 finalLinearImpulse{};
		glm::vec3 finalAngularImpulse{};

		Manifold &manifold  = manifoldPkg.manifolds[i];

		if (manifold.frictionRestitution.x <= 0.0f)
			manifold.frictionRestitution.x = 10.0f;

		if (manifold.frictionRestitution.y <= 0.0f)
			manifold.frictionRestitution.y = 0.010f;

		int referenceBoxIdx = manifold.contactBoxIndicesAndContactCount.x;
		int incidentBoxIdx  = manifold.contactBoxIndicesAndContactCount.y;

		LinearTransform &referenceLinearTransform   = getLinearTransform(referenceBoxIdx, rigidLinearTransformContainer, staticLinearTransformContainer);
		AngularTransform &referenceAngularTransform = getAngularTransform(referenceBoxIdx, rigidAngularTransformContainer, staticAngularTransformContainer);

		LinearTransform &incidentLinearTransform   = getLinearTransform(incidentBoxIdx, rigidLinearTransformContainer, staticLinearTransformContainer);
		AngularTransform &incidentAngularTransform = getAngularTransform(incidentBoxIdx, rigidAngularTransformContainer, staticAngularTransformContainer);

		// Compute the basis
		computeBasis(manifold.contactNormal, manifold.contactTangents[0], manifold.contactTangents[1]);

		glm::vec3 vA = referenceLinearTransform.velocity;
		glm::vec3 wA = referenceAngularTransform.angularVelocity;
		glm::vec3 vB = incidentLinearTransform.velocity;
		glm::vec3 wB = incidentAngularTransform.angularVelocity;

		// Iterate through each contact points
		for (int contactPointIdx = 0
			; contactPointIdx < manifold.contactBoxIndicesAndContactCount.z
			; ++contactPointIdx)
		{
			Contact &contact = manifold.contacts[contactPointIdx];

			glm::vec3 contactPointPos = manifold.contacts[contactPointIdx].position;

			// Relative positions of the contact point to the 2 bodies
			contact.referenceRelativePosition = contact.position - glm::vec4(referenceLinearTransform.position, 0.0f);
			contact.incidentRelativePosition  = contact.position - glm::vec4(incidentLinearTransform.position, 0.0f);

			// Precalculate J M^-1 JT for contact and friction constraint
			glm::vec3 referenceRelativePosCrossNormal = glm::cross(glm::vec3(contact.referenceRelativePosition), glm::vec3(manifold.contactNormal));
			glm::vec3 incidentRelativePosCrossNormal  = glm::cross(glm::vec3(contact.incidentRelativePosition), glm::vec3(manifold.contactNormal));

			float normalTotalInverseMass  = referenceLinearTransform.inverseMass + incidentLinearTransform.inverseMass;
			float tangentInverseMasses[2] = { normalTotalInverseMass, normalTotalInverseMass };

			normalTotalInverseMass += glm::dot(referenceRelativePosCrossNormal, referenceAngularTransform.inverseInertia * referenceRelativePosCrossNormal)
				+ glm::dot(incidentRelativePosCrossNormal, incidentAngularTransform.inverseInertia * incidentRelativePosCrossNormal);

			contact.normalTangentMassesBias.x = 1.0f / normalTotalInverseMass;

			// Compute the inverse masses in the 2 tangent components
			for (int j = 0; j < 2; ++j)
			{
				glm::vec3 referenceRelativePosCrossTangent = glm::cross(glm::vec3(manifold.contactTangents[j]), glm::vec3(contact.referenceRelativePosition));
				glm::vec3 incidentRelativePosCrossTangent  = glm::cross(glm::vec3(manifold.contactTangents[j]), glm::vec3(contact.incidentRelativePosition));
				tangentInverseMasses[j] += glm::dot(referenceRelativePosCrossTangent, referenceAngularTransform.inverseInertia * referenceRelativePosCrossTangent)
					+ glm::dot(incidentRelativePosCrossTangent, incidentAngularTransform.inverseInertia * incidentRelativePosCrossTangent);
				contact.normalTangentMassesBias[j + 1] = 1.0f / tangentInverseMasses[j];
			}

			// Precalculate the bias factor
			contact.normalTangentMassesBias.w = -cBaumgarteFactor * std::min(0.0f, manifold.contactNormal.w + cPenetrationSlop) / dt;

			// Warm start
			glm::vec3 oldP = glm::vec3(manifold.contactNormal) *contact.normalTangentBiasImpulses.x;

			// Friction
			oldP += glm::vec3(manifold.contactTangents[0]) * contact.normalTangentBiasImpulses.y;
			oldP += glm::vec3(manifold.contactTangents[1]) * contact.normalTangentBiasImpulses.z;

			vA -= oldP * referenceLinearTransform.inverseMass;
			wA -= referenceAngularTransform.inverseInertia * glm::cross(glm::vec3(contact.referenceRelativePosition), oldP);

			vB += oldP * incidentLinearTransform.inverseMass;
			wB += incidentAngularTransform.inverseInertia * glm::cross(glm::vec3(contact.incidentRelativePosition), oldP);

			// Restitution bias
			float dv = glm::dot(vB + glm::cross(wB, glm::vec3(contact.incidentRelativePosition)) - vA - glm::cross(wA, glm::vec3(contact.referenceRelativePosition))
				, glm::vec3(manifold.contactNormal));

			if (dv < -1.0f)
			{
				contact.normalTangentMassesBias.w += -(manifold.frictionRestitution.y) * dv;
			}

			referenceLinearTransform.velocity = vA;
			referenceAngularTransform.angularVelocity = wA;
			incidentLinearTransform.velocity = vB;
			incidentAngularTransform.angularVelocity = wB;
		}
	}
}

void P3ConstraintSolver::solve( ManifoldGpuPackage &manifoldPkg,
								std::vector<LinearTransform> &rigidLinearTransformContainer,
								std::vector<AngularTransform> &rigidAngularTransformContainer,
								std::vector<LinearTransform> &staticLinearTransformContainer,
								std::vector<AngularTransform> &staticAngularTransformContainer )
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

		// Solve constraint in pair, so 2 times every time if the 2 bodies are both rigid. This solver doesn't work on static bodies. However,
		//  it does use the info from the static bodies.

		// Forgive me for I have sinned
		LinearTransform const &referenceLinearTransform   = getLinearTransform(referenceBoxIdx, rigidLinearTransformContainer, staticLinearTransformContainer);
		AngularTransform const &referenceAngularTransform = getAngularTransform(referenceBoxIdx, rigidAngularTransformContainer, staticAngularTransformContainer);

		LinearTransform const &incidentLinearTransform   = getLinearTransform(incidentBoxIdx, rigidLinearTransformContainer, staticLinearTransformContainer);
		AngularTransform const &incidentAngularTransform = getAngularTransform(incidentBoxIdx, rigidAngularTransformContainer, staticAngularTransformContainer);

		// Iterate through each contact points
		for ( int contactPointIdx = 0
			; contactPointIdx < manifold.contactBoxIndicesAndContactCount.z
			; ++contactPointIdx )
		{
			glm::vec3 contactPointPos = manifold.contacts[contactPointIdx].position;

			// Relative positions of the contact point to the 2 bodies
			glm::vec3 r_1 = contactPointPos - referenceLinearTransform.position;
			glm::vec3 r_2 = contactPointPos - incidentLinearTransform.position;

			// Relative velocity at contact point
			glm::vec3 dv = incidentLinearTransform.velocity + glm::cross(incidentAngularTransform.angularVelocity, r_2)
						 - referenceLinearTransform.velocity + glm::cross(referenceAngularTransform.angularVelocity, r_1);

			// Find the tangent axis to the surface of the reference box - tangent component to simulate friction
			glm::vec3 tangent = glm::normalize(glm::cross(glm::vec3(manifold.contactNormal), incidentAngularTransform.angularVelocity));

			// Tangent component of delta v
			float vt = glm::dot(dv, tangent);

			// TODO: To be continue...

			glm::vec3 r = contactPointPos - incidentLinearTransform.position;

			r = glm::normalize(r);
			// Apply linear impulse onto each contact point
			finalLinearImpulse += 0.20f * (0.3f * glm::dot(-r, glm::vec3(manifold.contactNormal))
								+ 0.35f * (glm::length(incidentLinearTransform.velocity) + 0.15f)) * -r;

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

void P3ConstraintSolver::iterativeSolve( ManifoldGpuPackage &manifoldPkg,
										 std::vector<LinearTransform> &rigidLinearTransformContainer,
										 std::vector<AngularTransform> &rigidAngularTransformContainer,
										 std::vector<LinearTransform> &staticLinearTransformContainer,
										 std::vector<AngularTransform> &staticAngularTransformContainer )
{
	for (int iteration = 0; mResetCounter == 0 && iteration < cIterationCount; ++iteration)
	{
		for (int k = 0; k < manifoldPkg.misc.x; ++k)
		{
			Manifold &manifold = manifoldPkg.manifolds[k];

			int referenceBoxIdx = manifold.contactBoxIndicesAndContactCount.x;
			int incidentBoxIdx  = manifold.contactBoxIndicesAndContactCount.y;

			LinearTransform &referenceLinearTransform   = getLinearTransform(referenceBoxIdx, rigidLinearTransformContainer, staticLinearTransformContainer);
			AngularTransform &referenceAngularTransform = getAngularTransform(referenceBoxIdx, rigidAngularTransformContainer, staticAngularTransformContainer);

			LinearTransform &incidentLinearTransform   = getLinearTransform(incidentBoxIdx, rigidLinearTransformContainer, staticLinearTransformContainer);
			AngularTransform &incidentAngularTransform = getAngularTransform(incidentBoxIdx, rigidAngularTransformContainer, staticAngularTransformContainer);

			glm::vec3 vA = referenceLinearTransform.velocity;
			glm::vec3 wA = referenceAngularTransform.angularVelocity;
			glm::vec3 vB = incidentLinearTransform.velocity;
			glm::vec3 wB = incidentAngularTransform.angularVelocity;

			for (int contactPointIdx = 0; contactPointIdx < manifold.contactBoxIndicesAndContactCount.z; ++contactPointIdx)
			{
				Contact &contact = manifold.contacts[contactPointIdx];

				// Relative velocity at contact
				glm::vec3 dv = vB + glm::cross(wB, glm::vec3(contact.incidentRelativePosition)) - vA 
							 - glm::cross(wA, glm::vec3(contact.referenceRelativePosition));

				// For friction
				for (int k = 0; k < 2; ++k)
				{
					float lambda = -glm::dot(dv, glm::vec3(manifold.contactTangents[k])) * contact.normalTangentMassesBias[k + 1];

					// Frictional impulse
					float maxLambda = manifold.frictionRestitution.x * contact.normalTangentBiasImpulses.x;

					// Clamp frictional impulse
					float oldTangentImpulse = contact.normalTangentBiasImpulses[k + 1];
					contact.normalTangentBiasImpulses[k + 1] = glm::clamp(oldTangentImpulse + lambda, -maxLambda, maxLambda);
					lambda = contact.normalTangentBiasImpulses[k + 1] - oldTangentImpulse;

					// Apply frictional impulse
					glm::vec3 tangentImpulse = manifold.contactTangents[k] * lambda;
					vA -= tangentImpulse * referenceLinearTransform.inverseMass;
					wA -= referenceAngularTransform.inverseInertia * glm::cross(glm::vec3(contact.referenceRelativePosition), tangentImpulse);

					vB += tangentImpulse * incidentLinearTransform.inverseMass;
					wB += incidentAngularTransform.inverseInertia * glm::cross(glm::vec3(contact.incidentRelativePosition), tangentImpulse);
				}

				// Solve contact constraint
				dv = vB + glm::cross(wB, glm::vec3(contact.incidentRelativePosition)) - vA - glm::cross(wA, glm::vec3(contact.referenceRelativePosition));

				// Normal impulse
				float vn = glm::dot(dv, glm::vec3(manifold.contactNormal));

				// Factor in positional bias
				float lambda = contact.normalTangentMassesBias.x * (-vn + contact.normalTangentMassesBias.w);

				// Clamp impulse
				float tempNormalImpulse = contact.normalTangentBiasImpulses.x;
				contact.normalTangentBiasImpulses.x = std::max(tempNormalImpulse + lambda, 0.0f);
				lambda = contact.normalTangentBiasImpulses.x - tempNormalImpulse;

				// Apply impulse
				glm::vec3 normalImpulse = glm::vec3(manifold.contactNormal) * lambda;
				vA -= normalImpulse * referenceLinearTransform.inverseMass;
				wA -= referenceAngularTransform.inverseInertia * glm::cross(glm::vec3(contact.referenceRelativePosition), normalImpulse);

				vB += normalImpulse * incidentLinearTransform.inverseMass;
				wB += incidentAngularTransform.inverseInertia * glm::cross(glm::vec3(contact.incidentRelativePosition), normalImpulse);
			}

			// Directly apply the change in velocities
			referenceLinearTransform.velocity = vA;
			referenceAngularTransform.angularVelocity = wA;
			incidentLinearTransform.velocity = vB;
			incidentAngularTransform.angularVelocity = wB;
		}
	}
}
