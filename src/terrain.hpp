#pragma once

#include "cgp/cgp.hpp"

struct perlin_noise_parameters
{
	float persistency = 0.45f;     
	float frequency_gain = 2.0f;   
	int octave = 6;
	float terrain_height = 4.5f;  
};


cgp::mesh create_terrain_mesh();

void update_terrain(cgp::mesh& terrain, cgp::mesh_drawable& terrain_visual, perlin_noise_parameters const& parameters, std::vector<float> noisy_tab, float t);
