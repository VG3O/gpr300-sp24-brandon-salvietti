/*
	Animation // Brandon Salvietti

	Strings keyframes together to play a simple animation.
*/
#pragma once

#include "transform.h"

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
		Keyframe()
		{
			time = 0;
			value = glm::vec3(0, 0, 0);
		}

		Keyframe(float _time, glm::vec3 target)
		{
			time = _time;
			value = target;
		}

		float time = 0;
		glm::vec3 value;
		EasingStyle ease = LINEAR;
		bool easeIn = false;
	};

	class Animation
	{
	public:
		Animation() { animations.push_back(this); }
		~Animation()
		{
			auto it = std::find_if(animations.begin(), animations.end(), [this](Animation* animation) { return this == animation;});
			if (it == animations.end()) return;
			animations.erase(it);
		}
		
		void AddKeyframe(Keyframe keyframe) 
		{ 
			mKeyframes.push_back(keyframe); 
			UpdateDuration(); 
		}
		void AddKeyframes(std::vector<Keyframe> keyframes) 
		{ 
			mKeyframes.insert(mKeyframes.end(), keyframes.begin(), keyframes.end()); 
			UpdateDuration(); 
		
		}
		void PopKeyframe()
		{
			if (mKeyframes.size() == 0) return;
			mKeyframes.pop_back();
		}
		void RemoveKeyframe(int whereAt)
		{
			if (whereAt + 1 > mKeyframes.size() || mKeyframes.size() == 0) return;
			mKeyframes.erase(mKeyframes.begin()+whereAt);
		}

		void ClearKeyframes() { mKeyframes.clear(); UpdateDuration(); }
		std::vector<Keyframe> GetKeyframes() { return mKeyframes; }
		float GetDuration() { return mDuration; }

		static void Cleanup() 
		{
			for (int i = 0; i < animations.size(); i++)
				delete animations[i];
			
			animations.clear();
		};
	private:
		static std::vector<Animation*> animations;

		void UpdateDuration();
		std::vector<Keyframe> mKeyframes;
		float mDuration = 0; // maximal keyframe time, set this on keyframe change
	};

	class Animator
	{
	public:
		void Play() { playing = true; }
		void Stop() { playing = false; playbackTime = 0; }
		void Pause() { playing = false; }
		void Loop(bool state) { looping = state; }

		void SetAnimation(Animation* newAnimation, int type) 
		{ 
			playing = false; 
			switch (type)
			{
			case 1:
				mPosAnimation = newAnimation;
				break;
			case 2:
				mRotAnimation = newAnimation;
				break;
			case 3:
				mScaleAnimation = newAnimation;
				break;
			}
		}
		Animation* GetAnimation(int type) 
		{
			switch (type)
			{
			case 1:
				return mPosAnimation;
			case 2:
				return mRotAnimation;
			case 3:
				return mScaleAnimation;
			}
			return nullptr;
		}
		ew::Transform UpdateAnimations(float dt);
	private:

		Animation* mPosAnimation = nullptr;
		Animation* mScaleAnimation = nullptr;
		Animation* mRotAnimation = nullptr;

		bool playing = false;
		bool looping = false;
		float maximalDuration = 0;
		float playbackSpeed = 1;
		float playbackTime = 0;
	};
}