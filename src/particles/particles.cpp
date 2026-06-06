#include "particles.hpp"
#include <random>

using namespace cgp;

particle_snowflake::particle_snowflake(float L)
{
	float const x = rand_uniform(-L, L);
	float const y = rand_uniform(-L, L);
	float const z = rand_uniform(0, max_height);
	p0 = vec3(x, y, z);
	size = rand_uniform(0.5, 1.5);
}