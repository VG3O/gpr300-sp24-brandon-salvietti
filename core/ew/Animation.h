/*
	Animation // Brandon Salvietti

	Strings keyframes together to play a simple animation.
*/
#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>

namespace vg3o
{
	typedef float (*EaseFunction)(float, bool);

	enum EasingStyle
	{
		LINEAR,
		QUADRATIC,
		CUBIC,
		QUARTIC,
		QUINTIC,
		EXPONENTIAL,
		SINE,
		BACK,
		CIRCULAR
	};

	struct Keyframe
	{
		float time;
		glm::vec3 value;
		EasingStyle ease;
		bool easeIn = false;
	};

	class Animation
	{
	public:
		void AddKeyframe(Keyframe keyframe);
		void AddKeyframes(std::vector<Keyframe> keyframes);
		void ClearKeyframes();
	private:
		void UpdateDuration();
		std::vector<Keyframe> keyframes;
		float duration; // maximal keyframe time, set this on keyframe change
	};

	class Animator
	{
	public:
		void Play();
		void Stop();
		void Pause();
		void UpdateAnimations(float dt);
	private:
		bool playing = false;
		bool looping = false;
		float playbackSpeed = 1;
		float playbackTime = 0;
	};
}