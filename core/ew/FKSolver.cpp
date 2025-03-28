#include "FKSolver.h"

using namespace vg3o;

void SolveFK(Joint* joint)
{
	if (joint->parent == nullptr)
	{
		joint->globalPose = joint->pose.modelMatrix();
	}
	else
	{
		joint->globalPose = joint->parent->globalPose * joint->pose.modelMatrix();
	}
	for (auto& child : joint->children)
		SolveFK(child);
}