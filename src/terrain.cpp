
#include "terrain.hpp"
#include <random>

using namespace cgp;
std::random_device rd;
std::mt19937 gen(rd());

mesh create_terrain_mesh()
{
	int const terrain_sample = 600;
	mesh terrain = mesh_primitive_grid({ -10,-10,0 }, { 10,-10,0 }, { 10,10,0 }, { -10,10,0 }, terrain_sample, terrain_sample);
	return terrain;
}

void update_terrain(mesh& terrain, mesh_drawable& terrain_visual, perlin_noise_parameters const& parameters, std::vector<float> noisy_tab, float t)
{
	int const N = std::sqrt(terrain.position.size());

	for (int ku = 0; ku < N; ++ku) {
		for (int kv = 0; kv < N; ++kv) {

			const float u = ku / (N - 1.0f);
			const float v = kv / (N - 1.0f);

			int const idx = ku * N + kv;

			float const nx = u * 2.0f - 1.0f;
			float const ny = v * 2.0f - 1.0f;

			// modulation par une gaussienne

			float const sigma = 0.4f;
			float const distance_sq = nx * nx + ny * ny;
			float const gaussian = std::exp(-distance_sq / (2.0f * sigma * sigma));
			float const base_mask = 1.0f - gaussian;
			float const mask = base_mask * base_mask;

			// bruit de perlin
			float const raw_noise = noise_perlin({ u * 3.0f, v * 3.0f }, parameters.octave, parameters.persistency, parameters.frequency_gain);

			float const noise = raw_noise * mask;

			terrain.position[idx].z = parameters.terrain_height * noise;

		}
	}

	terrain.normal_update();

	terrain_visual.vbo_position.update(terrain.position);
	terrain_visual.vbo_normal.update(terrain.normal);
	//terrain_visual.vbo_color.update(terrain.color);

}

float terrain_height(mesh& terrain, float x, float y)
{
	int const N = std::sqrt(terrain.position.size());
	int ku = static_cast<int>((x + 10.0f) / 20.0f * (N - 1));
	int kv = static_cast<int>((y + 10.0f) / 20.0f * (N - 1));
	int idx = ku * N + kv;
	return terrain.position[idx].z;
}