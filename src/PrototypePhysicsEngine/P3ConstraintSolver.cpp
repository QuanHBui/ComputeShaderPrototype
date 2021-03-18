#include "P3ConstraintSolver.h"

#include "P3Collider.h"
#include "P3NarrowPhaseCommon.h"
#include "P3Transform.h"

constexpr float cBaumgarteFactor = 0.001f;
constexpr float cPenetrationSlop = 0.005f;
constexpr int cIterationCount = 20;

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
		Manifold &manifold  = manifoldPkg.manifolds[i];

		if (manifold.frictionRestitution.x <= 0.0f)
			manifold.frictionRestitution.x = 1.0f;

		if (manifold.frictionRestitution.y <= 0.0f)
			manifold.frictionRestitution.y = 0.05f;

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

			// Relative positions of the contact point to the 2 bodies
			contact.referenceRelativePosition = contact.position - referenceLinearTransform.position;
			contact.incidentRelativePosition  = contact.position - incidentLinearTransform.position;

			// Precalculate J M^-1 JT for contact and friction constraint
			glm::vec3 referenceRelativePosCrossNormal = glm::cross(glm::vec3(contact.referenceRelativePosition), glm::vec3(manifold.contactNormal));
			glm::vec3 incidentRelativePosCrossNormal  = glm::cross(glm::vec3(contact.incidentRelativePosition), glm::vec3(manifold.contactNormal));

			float normalTotalInverseMass  = referenceLinearTransform.inverseMass + incidentLinearTransform.inverseMass;
			float tangentInverseMasses[2] = { normalTotalInverseMass, normalTotalInverseMass };

			normalTotalInverseMass += glm::dot(referenceRelativePosCrossNormal, glm::mat3(referenceAngularTransform.inverseInertia) * referenceRelativePosCrossNormal)
				+ glm::dot(incidentRelativePosCrossNormal, glm::mat3(incidentAngularTransform.inverseInertia) * incidentRelativePosCrossNormal);

			contact.normalTangentMassesBias.x = 1.0f / normalTotalInverseMass;

			// Compute the inverse masses in the 2 tangent components
			for (int j = 0; j < 2; ++j)
			{
				glm::vec3 referenceRelativePosCrossTangent = glm::cross(glm::vec3(manifold.contactTangents[j]), glm::vec3(contact.referenceRelativePosition));
				glm::vec3 incidentRelativePosCrossTangent  = glm::cross(glm::vec3(manifold.contactTangents[j]), glm::vec3(contact.incidentRelativePosition));
				tangentInverseMasses[j] += glm::dot(referenceRelativePosCrossTangent, glm::mat3(referenceAngularTransform.inverseInertia) * referenceRelativePosCrossTangent)
					+ glm::dot(incidentRelativePosCrossTangent, glm::mat3(incidentAngularTransform.inverseInertia) * incidentRelativePosCrossTangent);
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
			wA -= glm::mat3(referenceAngularTransform.inverseInertia) * glm::cross(glm::vec3(contact.referenceRelativePosition), oldP);

			vB += oldP * incidentLinearTransform.inverseMass;
			wB += glm::mat3(incidentAngularTransform.inverseInertia) * glm::cross(glm::vec3(contact.incidentRelativePosition), oldP);

			// Restitution bias
			float dv = glm::dot(vB + glm::cross(wB, glm::vec3(contact.incidentRelativePosition)) - vA - glm::cross(wA, glm::vec3(contact.referenceRelativePosition))
				, glm::vec3(manifold.contactNormal));

			if (dv < -1.0f)
			{
				contact.normalTangentMassesBias.w += -(manifold.frictionRestitution.y) * dv;
			}

			referenceLinearTransform.velocity = glm::vec4(vA, 0.0f);
			referenceAngularTransform.angularVelocity = glm::vec4(wA, 0.0f);
			incidentLinearTransform.velocity = glm::vec4(vB, 0.0f);
			incidentAngularTransform.angularVelocity = glm::vec4(wB, 0.0f);
		}
	}
}

void P3ConstraintSolver::iterativeSolve( ManifoldGpuPackage &manifoldPkg,
										 std::vector<LinearTransform> &rigidLinearTransformContainer,
										 std::vector<AngularTransform> &rigidAngularTransformContainer,
										 std::vector<LinearTransform> &staticLinearTransformContainer,
										 std::vector<AngularTransform> &staticAngularTransformContainer )
{
	for (int iteration = 0; iteration < cIterationCount; ++iteration)
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
					wA -= glm::mat3(referenceAngularTransform.inverseInertia) * glm::cross(glm::vec3(contact.referenceRelativePosition), tangentImpulse);

					vB += tangentImpulse * incidentLinearTransform.inverseMass;
					wB += glm::mat3(incidentAngularTransform.inverseInertia) * glm::cross(glm::vec3(contact.incidentRelativePosition), tangentImpulse);
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
				wA -= glm::mat3(referenceAngularTransform.inverseInertia) * glm::cross(glm::vec3(contact.referenceRelativePosition), normalImpulse);

				vB += normalImpulse * incidentLinearTransform.inverseMass;
				wB += glm::mat3(incidentAngularTransform.inverseInertia) * glm::cross(glm::vec3(contact.incidentRelativePosition), normalImpulse);
			}

			// Directly apply the change in velocities
			referenceLinearTransform.velocity = glm::vec4(vA, 0.0f);
			referenceAngularTransform.angularVelocity = glm::vec4(wA, 0.0f);
			incidentLinearTransform.velocity = glm::vec4(vB, 0.0f);
			incidentAngularTransform.angularVelocity = glm::vec4(wB, 0.0f);
		}
	}
}
