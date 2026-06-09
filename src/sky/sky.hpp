#pragma once

#include "cgp/cgp.hpp"

struct light_source // Attributes of a light source
{
	cgp::vec3 position;
	cgp::vec3 color;
	float phong_ambient;
	float phong_diffuse;
	float phong_specular;
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