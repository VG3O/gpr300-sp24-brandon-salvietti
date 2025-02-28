#include "Animation.h"
#include <cmath>

float PI = 3.14159265359;
float CLAMP(float value, float lower, float upper) { return std::max(lower, std::min(value, upper)); }

/*
----
Easing Functions
----
*/

float interpolate(float a, float b, float t)
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

/*
----
Animation
----
*/

std::unordered_map<vg3o::EasingStyle, vg3o::EaseFunction> timeFunctions = {
	{vg3o::LINEAR, linear},
	{vg3o::QUADRATIC, quad},
	{vg3o::CUBIC, cubic},
	{vg3o::QUARTIC, quart},
	{vg3o::QUINTIC, quint},
	{vg3o::SINE, sine},
	{vg3o::EXPONENTIAL, exponential},
	{vg3o::BACK, back},
	{vg3o::CIRCULAR, circular},
};

