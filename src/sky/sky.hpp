#pragma once

#include "cgp/cgp.hpp"

struct light_source // Attributes of a light source
{
	static const cgp::vec3 night_light;
	static const cgp::vec3 sunrise_light;
	static const cgp::vec3 day_light ;

	static const cgp::vec3 night_pos;
	static const cgp::vec3 sunrise_pos;
	static const cgp::vec3 day_pos;

	cgp::vec3 position;
	cgp::vec3 color;
	
	light_source();

	void update_light(float t);

};

struct sky_color
{
	static const cgp::vec3 zenith_midnight;
	static const cgp::vec3 horizon_midnight;
	static const cgp::vec3 zenith_sunrise;
	static const cgp::vec3 horizon_sunrise;
	static const cgp::vec3 zenith_noon;
	static const cgp::vec3 horizon_noon;

	cgp::vec3 zenith_color;
	cgp::vec3 horizon_color;
	sky_color();

	void update_color(float t);
};