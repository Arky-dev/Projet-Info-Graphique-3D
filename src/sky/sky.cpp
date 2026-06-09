#include "sky.hpp"
#include <math.h>

const cgp::vec3 sky_color::zenith_midnight = {0.0f, 0.0f, 0.2f};
const cgp::vec3 sky_color::horizon_midnight = {0.1f, 0.1f, 0.1f};
const cgp::vec3 sky_color::zenith_sunrise = { 0.2f, 0.2f, 0.5f };
const cgp::vec3 sky_color::horizon_sunrise = {1.0f, 0.7f, 0.2f};
const cgp::vec3 sky_color::zenith_noon = {0.5f, 0.5f, 0.8f};
const cgp::vec3 sky_color::horizon_noon = {0.5f, 0.5f, 1.0f};

sky_color::sky_color()
{
	zenith_color = zenith_noon;
	horizon_color = horizon_noon;
}

void sky_color::update_color(float t)
{
	float t_day = fmod(t, 24.0f) ;
	if (t_day < 6.0f) {
		float lambda = t_day / 6.0f;
		zenith_color = (1 - lambda) * zenith_midnight + lambda * zenith_sunrise;
		horizon_color = (1 - lambda) * horizon_midnight + lambda * horizon_sunrise;
	}
	else if (t_day < 12.0f) {
		float lambda = (t_day - 6.0f) / 6.0f;
		zenith_color = (1 - lambda) * zenith_sunrise + lambda * zenith_noon;
		horizon_color = (1 - lambda) * horizon_sunrise + lambda * horizon_noon;
	}
	else if (t_day < 18.0f) {
		float lambda = (t_day - 12.0f) / 6.0f;
		zenith_color = (1 - lambda) * zenith_noon + lambda * zenith_sunrise;
		horizon_color = (1 - lambda) * horizon_noon + lambda * horizon_sunrise;
	}
	else {
		float lambda = (t_day - 18.0f) / 6.0f;
		zenith_color = (1 - lambda) * zenith_sunrise + lambda * zenith_midnight;
		horizon_color = (1 - lambda) * horizon_sunrise + lambda * horizon_midnight;
	}
}