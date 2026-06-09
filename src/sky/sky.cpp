#include "sky.hpp"
#include <math.h>

// Sky colors
const cgp::vec3 sky_color::zenith_midnight = {0.0f, 0.0f, 0.2f};
const cgp::vec3 sky_color::horizon_midnight = {0.1f, 0.1f, 0.1f};
const cgp::vec3 sky_color::zenith_sunrise = { 0.2f, 0.2f, 0.5f };
const cgp::vec3 sky_color::horizon_sunrise = {1.0f, 0.7f, 0.2f};
const cgp::vec3 sky_color::zenith_noon = {0.5f, 0.5f, 0.8f};
const cgp::vec3 sky_color::horizon_noon = {0.5f, 0.5f, 1.0f};

// Light colors
const cgp::vec3 light_source::night_light = { 0.8f, 0.8f, 0.8f };
const cgp::vec3 light_source::sunrise_light = { 1.0f, 0.5f, 0.5f };
const cgp::vec3 light_source::day_light = { 1.0f, 0.9f, 0.9f };

// Light positions
const cgp::vec3 light_source::night_pos = { -3.0f, 3.0f, 10.0f };
const cgp::vec3 light_source::sunrise_pos= { 0.0f, 5.0f, 4.0f };
const cgp::vec3 light_source::day_pos = { 3.0f, 3.0f, 10.0f };

sky_color::sky_color()
{
	zenith_color = zenith_midnight;
	horizon_color = horizon_midnight;
}

void sky_color::update_color(float t)
{
	if (t < 0.5f) {
		float lambda = 2.0f * t;
		zenith_color = (1 - lambda) * zenith_midnight + lambda * zenith_sunrise;
		horizon_color = (1 - lambda) * horizon_midnight + lambda * horizon_sunrise;
	}
	else {
		float lambda = 2.0f * (t - 0.5f);
		zenith_color = (1 - lambda) * zenith_sunrise + lambda * zenith_noon;
		horizon_color = (1 - lambda) * horizon_sunrise + lambda * horizon_noon;
	}
	
}

light_source::light_source()
{
	color = night_light;
	position = night_pos;
}

void light_source::update_light(float t)
{
	if (t < 0.5f) {
		float lambda = 2.0f * t;
		color = night_light;
		position = (1 - lambda) * night_pos + lambda * sunrise_pos;
	}
	else {
		float lambda = 2.0f * (t - 0.5f);
		color = (1 - lambda) * sunrise_light + lambda * day_light;
		position = (1 - lambda) * sunrise_pos + lambda * day_pos;
	}
}