#pragma once

#include "transform.h"
#include <glm//glm.hpp>
#include <string>
#include <vector>

namespace vg3o {
	struct Joint {
		Joint(std::string name, glm::vec3 pos, glm::vec3 rot = glm::vec3(0.f, 0.f, 0.f), glm::vec3 scale = glm::vec3(1.f, 1.f, 1.f)) {
			pose.position = pos;
			pose.rotation = glm::quat(rot);
			pose.scale = scale;
			jointName = name;
		}

		ew::Transform pose; // local transform
		glm::mat4 globalPose; // global pose
		
		std::string jointName;
		Joint* parent;
		std::vector<Joint*> children;
	};

	struct Skeleton {
		Joint* root; 
	};

	void SolveFK(Skeleton* skeleton, Joint* joint);
}