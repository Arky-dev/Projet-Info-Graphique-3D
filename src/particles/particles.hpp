#pragma once

#include "cgp/cgp.hpp"


struct particle_snowflake
{
	cgp::vec3 p0;  // Initial position
	float t0;      // Time of birth
	cgp::vec3 p; // Current position (updated at each frame)
	cgp::vec3 color;
	float size;

	particle_snowflake(float creation_time = 0);               // Create a snowflake at its initial position
	cgp::vec3 evaluate_position(float absolute_time); // Evaluate its position at a given time
};


// Storage of all the particles
struct particle_system_structure
{
	std::vector<particle_snowflake> snowflakes;
	cgp::numarray<cgp::vec4> snowflake_positions_and_sizes; // Container to store the positions of the snowflakes (used for the shader)
	void evaluate_all_positions(float absolute_time);

	void remove_old_particles(float current_time);
};