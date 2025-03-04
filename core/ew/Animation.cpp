#include "Animation.h"
#include <cmath>

float PI = 3.14159265359;
float CLAMP(float value, float lower, float upper) { return std::max(lower, std::min(value, upper)); }

glm::quat eulerToQuat(glm::vec3 euler)
{
	glm::quat q;
	
	float x = glm::radians(euler.x), y = glm::radians(euler.y), z = glm::radians(euler.z);
	q.w = cos(x / 2) * cos(y / 2) * cos(z / 2) + sin(x / 2) * sin(y / 2) * sin(z / 2);
	q.x = sin(x / 2) * cos(y / 2) * cos(z / 2) - cos(x / 2) * sin(y / 2) * sin(z / 2);
	q.y = cos(x / 2) * sin(y / 2) * cos(z / 2) + sin(x / 2) * cos(y / 2) * sin(z / 2);
	q.z = cos(x / 2) * cos(y / 2) * sin(z / 2) - sin(x / 2) * sin(y / 2) * cos(z / 2);

	return q;
}

/*
----
Easing Functions
----
*/

glm::vec3 interpolate(glm::vec3 a, glm::vec3 b, float t)
{
	t = CLAMP(t, 0, 1);
	return a + (b - a) * t;
}

// basic time functions
float linear(float t, bool easeIn) { return easeIn ? t : 1 - t; }
float quad(float t, bool easeIn) { return easeIn ? pow(t, 2) : 1 - pow(1-t, 2); }
float cubic(float t, bool easeIn) { return easeIn ? pow(t, 3) : 1 - pow(1-t, 3); }
float quart(float t, bool easeIn) { return easeIn ? pow(t, 4) : 1 - pow(1 - t, 4); }
float quint(float t, bool easeIn) { return easeIn ? pow(t, 5) : 1 - pow(1 - t, 5); }
float sine(float t, bool easeIn) { return easeIn ? (1 - cos((t * PI) / 2)) : sin((t * PI) / 2); }

// more complex time functions
float exponential(float t, bool easeIn) 
{ 
	return easeIn ? (t == 0.f ? 0.f : pow(2.f, 10.f * t - 10.f)) 
				   : (t == 1.f ? 1.f : 1.f - pow(2.f, -10 * t));
}
float back(float t, bool easeIn)
{
	float c1 = 1.70158f;
	float c3 = c1 + 1.f;
	return easeIn ? (c3 * t * t * t - c1 * t * t) : 1 + c3 * pow(t - 1.f, 3) + c1 * pow(t - 1.f, 2);
}
float circular(float t, bool easeIn)
{
	return easeIn ? (1.f - sqrt(1.f - pow(t, 2.f))) : sqrt(1.f - pow(t - 1.f, 2.f));
}

namespace vg3o {
	/*
	----
	Animation
	----
	*/
	std::unordered_map<vg3o::EasingStyle, vg3o::EaseFunction> timeFunctions = {
		{LINEAR, linear},
		{QUADRATIC, quad},
		{CUBIC, cubic},
		{QUARTIC, quart},
		{QUINTIC, quint},
		{SINE, sine},
		{EXPONENTIAL, exponential},
		{BACK, back},
		{CIRCULAR, circular},
	};
	
	std::vector<Animation*> Animation::animations;

	void Animation::UpdateDuration()
	{
		if (mKeyframes.size() == 0) { mDuration = 0; return; }

		for (int i = 0; i < mKeyframes.size(); i++)
			mDuration = std::max(mDuration, mKeyframes[i].time);
	}

	/*
	----
	Animator
	----
	*/

	ew::Transform Animator::UpdateAnimations(float dt)
	{
		if (playing)
		{
			dt *= playbackSpeed;
			playbackTime += dt;
		}
		ew::Transform newTransform;
		float maxAnimDuration = 0;
		for (int i = 1; i <= 3; i++)
		{
			Animation* animation = GetAnimation(i);
			if (animation == nullptr) continue;
			maxAnimDuration = std::max(maxAnimDuration, animation->GetDuration());

			std::vector<Keyframe> keyframes = animation->GetKeyframes();
			if (keyframes.size() <= 1) continue;

			Keyframe upper, lower;

			for (int i = 0; i < keyframes.size(); i++)
			{
				if (keyframes[i].time < playbackTime) continue;
				upper = keyframes[i];
				lower = keyframes[i - 1];
				break;
			}

			float difference = upper.time - lower.time;
			float time = (playbackTime - lower.time) / difference;
			time = timeFunctions[lower.ease](time, lower.easeIn);

			glm::vec3 value = interpolate(lower.value, upper.value, time);
			
			switch (i)
			{
			case 1:
				newTransform.position = value;
				break;
			case 2:
				newTransform.rotation = eulerToQuat(value);
				break;
			case 3:
				newTransform.scale = value;
				break;
			}
		}
		
		maximalDuration = maxAnimDuration;
		return newTransform;
	}
};

