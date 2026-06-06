#pragma once

#include "cgp/cgp.hpp"

struct particle_snowflake
{
	cgp::vec3 p0;  // Initial position
	float size;

	particle_snowflake(float L);               // Create a snowflake at its initial position
};


// Storage of all the particles
struct particle_system_structure
{
	std::vector<particle_snowflake> snowflakes;
	cgp::numarray<cgp::vec4> snowflake_positions_and_sizes; // Container to store the positions of the snowflakes (used for the shader)
};