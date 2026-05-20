#include "particles.hpp"

using namespace cgp;
particle_snowflake::particle_snowflake(float creation_time)
{
	t0 = creation_time;
	float max_height = 5.0f;
	float L = 5.0f;
	float const x = rand_uniform(-L, L);
	float const y = rand_uniform(-L, L);
	float const z = rand_uniform(0.0f, max_height);
	p0 = vec3(x, y, z);
	p = p0;
	size = rand_uniform(0.5, 1.5);
	color = { 1.0f, 1.0f, 1.0f };
}

// Evaluate the position of the snowflake at a given time
//  absolute_time is the current time since the beginning of the program
vec3 particle_snowflake::evaluate_position(float absolute_time)
{
	float t = absolute_time - t0;
	float s = 0.01f * size; // Speed of the snowflake
	if (p.z > 0.0f) {
		p = p + s * vec3(0.0f, 0.0f, -1.0f);
	}
	else {
		p.z = 5.0f; // Reset the snowflake to the top when it reaches the ground
	}
	return p;
}

// Initialize the position of the billboard at the time of its creation


void particle_system_structure::evaluate_all_positions(float absolute_time)
{
	int const N = snowflakes.size();
	snowflake_positions_and_sizes.resize(N);

	for (int k = 0; k < N; ++k)
	{
		// Current particle
		particle_snowflake& particle = snowflakes[k];
		//Evaluate the current position of the particle
		vec3 const p = particle.evaluate_position(absolute_time);
		snowflake_positions_and_sizes[k] = vec4{ p, particle.size };
	}
}

template <typename T>
static void remove_old_element(std::vector<T>& container, float current_time, float max_time)
{
	// Loop over all active particles
	for (auto it = container.begin(); it != container.end();)
	{
		// if a particle is too old, remove it
		if (current_time - it->t0 > max_time)
			it = container.erase(it);

		// Go to the next particle if we are not already on the last one
		if (it != container.end())
			++it;
	}
}


void particle_system_structure::remove_old_particles(float t)
{
	remove_old_element(snowflakes, t, 3.0f);
}