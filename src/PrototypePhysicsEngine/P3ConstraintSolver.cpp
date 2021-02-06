#include "P3ConstraintSolver.h"

#include "P3NarrowPhaseCommon.h"
#include "P3Transform.h"

std::vector<glm::vec3> const &P3ConstraintSolver::solve(
	ManifoldGpuPackage const &manifoldPkg,
	std::vector<LinearTransform> const &rigidLinearTransformContainer )
{
	// Gotta decide on which constraint to solve first: Floor or Contact
	for (LinearTransform const &linearTransform : rigidLinearTransformContainer)
	{
		glm::vec3 finalImpulse{};

		// Floor contraint
		if (rigidLinearTransformContainer[0].position.y <= -3.0f)
		{
			finalImpulse -= rigidLinearTransformContainer[0].velocity;
		}

		// This is utterly stupid.
		if (!mImpulseContainer.empty())
		{
			mImpulseContainer.pop_back();
		}
		mImpulseContainer.emplace_back(finalImpulse);
	}

	// Iterate through all manifolds
	for (int i = 0; i < manifoldPkg.misc.x; ++i)
	{
		glm::vec3 finalImpulse{};
		// A manifold contains collision info of a pair of objects, namely their handles/IDs.
		//  And we can use the IDs to access the component container.
		Manifold const &manifold = manifoldPkg.manifolds[i];
		int referenceBoxIdx = manifold.contactBoxIndicesAndContactCount.x; // We don't know what type the object is based on their IDs
		int incidentBoxIdx  = manifold.contactBoxIndicesAndContactCount.y;

		// Solve constraint in pair, so 2 times every time

		// Static contact constraint
		//finalImpulse += 0.65f * manifold.contactNormal.w * glm::vec3(manifold.contactNormal);
		finalImpulse += rigidLinearTransformContainer[0].velocity * glm::vec3(manifold.contactNormal);

		// This is freaking stupid.
		mImpulseContainer.back() += finalImpulse;
	}

	return mImpulseContainer;
}

void P3ConstraintSolver::solve()
{

}