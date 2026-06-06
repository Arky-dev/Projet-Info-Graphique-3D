#pragma once

#include "cgp/cgp.hpp"

struct perlin_noise_parameters
{
	float persistency = 0.45f;     // Rendu plus rocheux/escarpé
	float frequency_gain = 2.0f;   // On ne change pas (standard)
	int octave = 6;                // Un peu plus de micro-détails
	float terrain_height = 4.5f;   // Des vraies montagnes sur les bords
};


// Initialize the mesh of the terrain
cgp::mesh create_terrain_mesh();

// Recompute the vertices of the terrain everytime a parameter is modified
//  and update the mesh_drawable accordingly
void update_terrain(cgp::mesh& terrain, cgp::mesh_drawable& terrain_visual, perlin_noise_parameters const& parameters, std::vector<float> noisy_tab, float t);
