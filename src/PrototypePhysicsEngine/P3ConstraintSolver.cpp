#include "P3ConstraintSolver.h"

#include "P3NarrowPhaseCommon.h"
#include "P3Transform.h"

std::vector<glm::vec3> const &P3ConstraintSolver::solve(
	ManifoldGpuPackage const &manifoldPkg,
	std::vector<LinearTransform> const &rigidLinearTransformContainer )
{
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

		// Floor contraint
		if (rigidLinearTransformContainer[0].position.y <= -3.0f)
		{
			finalImpulse -= rigidLinearTransformContainer[0].velocity;
		}
		else // Static contact constraint
		{
			finalImpulse += 0.65f * manifold.contactNormal.w * glm::vec3(manifold.contactNormal);
		}

		if (!mImpulseContainer.empty())
		{
			mImpulseContainer.pop_back();
		}
		mImpulseContainer.emplace_back(finalImpulse);
	}

	return mImpulseContainer;
}

void P3ConstraintSolver::solve()
{

}