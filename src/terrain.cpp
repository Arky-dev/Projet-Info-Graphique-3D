
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
	// Number of samples in each direction (assuming a square grid)
	int const N = std::sqrt(terrain.position.size());

	// Recompute the new vertices
	for (int ku = 0; ku < N; ++ku) {
		for (int kv = 0; kv < N; ++kv) {

			// Compute local parametric coordinates (u,v) \in [0,1]
			const float u = ku / (N - 1.0f);
			const float v = kv / (N - 1.0f);

			int const idx = ku * N + kv;

			float const nx = u * 2.0f - 1.0f;
			float const ny = v * 2.0f - 1.0f;

			// 2. Calcul de la Gaussienne
			// 'sigma' contrôle la taille de la zone plate au centre. 
			// Plus sigma est petit, plus le creux est concentré au milieu.
			// ... (calcul de u, v, nx, ny)

			float const sigma = 0.4f;
			float const distance_sq = nx * nx + ny * ny;
			float const gaussian = std::exp(-distance_sq / (2.0f * sigma * sigma));

			// 3. Masque agressif : on l'élève au carré (ou cube) 
			// Cela garde le centre encore plus plat et fait monter les bords plus brutalement
			float const base_mask = 1.0f - gaussian;
			float const mask = base_mask * base_mask;	

			// 4. Bruit de Perlin haute fréquence
			// En multipliant u et v par 3.0f, on "dézoome", ce qui crée plusieurs 
			// sommets de montagnes au lieu d'une seule grosse bosse.
			float const raw_noise = noise_perlin({ u * 3.0f, v * 3.0f }, parameters.octave, parameters.persistency, parameters.frequency_gain);

			// 5. Modulation finale
			float const noise = raw_noise * mask;

			// Assurez-vous que parameters.terrain_height est assez élevé (ex: 5.0 ou 6.0)
			terrain.position[idx].z = parameters.terrain_height * noise;

		}
	}

	// Update the normal of the mesh structure
	terrain.normal_update();

	// Update step: Allows to update a mesh_drawable without creating a new one
	terrain_visual.vbo_position.update(terrain.position);
	terrain_visual.vbo_normal.update(terrain.normal);
	//terrain_visual.vbo_color.update(terrain.color);

}

// mesh const& terrain_base, mesh & snow_mesh, mesh_drawable & snow_visual, perlin_noise_parameters const& parameters, float t
void temp_terrain_update(mesh const& terrain_base, mesh& snow_mesh, mesh_drawable& snow_visual, perlin_noise_parameters const& parameters, float t) {

	int const N = std::sqrt(snow_mesh.position.size());

	static float last_t = t;
	float dt = t - last_t;
	if (dt <= 0.0f) return; // Si pas de temps écoulé, on ne fait rien
	last_t = t;

	static std::vector<float> snow_thickness(snow_mesh.position.size(), 0.0f);

	float const max_snow = 0.1f;
	float const snow_rate = 0.02f;
	float const wind_speed = 0.2f;
	float const offset_depart = -0.05f;

	bool mesh_needs_gpu_update = false; // Détecte si on doit envoyer à la carte graphique

	for (int ku = 0; ku < N; ++ku) {
		for (int kv = 0; kv < N; ++kv) {
			int const idx = ku * N + kv;

			// On lit la pente précalculée
			float pente = terrain_base.normal[idx].z;

			// OPTIMISATION 1 : Le "Early Exit"
			// On ne calcule le bruit QUE si le terrain est plat ET que la neige n'a pas atteint le maximum
			if (pente > 0.6f && snow_thickness[idx] < max_snow) {

				const float u = ku / (N - 1.0f);
				const float v = kv / (N - 1.0f);
				vec2 const offset = { u + t * wind_speed, v + t * wind_speed };

				// Le calcul lourd n'est appelé que si c'est strictement nécessaire
				float noise = noise_perlin(offset, parameters.octave, parameters.persistency, parameters.frequency_gain);
				float snow_fall = std::max(0.0f, noise);

				float increment = snow_fall * snow_rate * dt;

				if (increment > 0.0f) {
					if (snow_thickness[idx] + increment > max_snow) {
						increment = max_snow - snow_thickness[idx];
					}

					snow_thickness[idx] += increment;
					snow_mesh.position[idx].z = terrain_base.position[idx].z + offset_depart + snow_thickness[idx];

					mesh_needs_gpu_update = true; // On a modifié au moins un sommet
				}
			}
		}
	}

	// OPTIMISATION 2 : On supprime l'appel destructeur de performances :
	// snow_mesh.normal_update(); 

	// OPTIMISATION 3 : On évite d'engorger le bus de la carte graphique
	if (mesh_needs_gpu_update) {
		snow_visual.vbo_position.update(snow_mesh.position);
		// Plus besoin de mettre à jour le VBO des normales !
	}
}

