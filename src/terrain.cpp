
#include "terrain.hpp"
#include <random>

using namespace cgp;
std::random_device rd;
std::mt19937 gen(rd());

mesh create_terrain_mesh()
{
	int const terrain_sample = 180;
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

void temp_terrain_update(mesh& terrain, mesh_drawable& terrain_visual, perlin_noise_parameters const& parameters, std::vector<float> noisy_tab, float t) {

	int const N = std::sqrt(terrain.position.size());

	// 1. Calcul du Delta Time (dt) pour une croissance constante
	// On stocke le temps de la frame précédente pour savoir combien de temps s'est écoulé
	static float last_t = t;
	float dt = t - last_t;
	if (dt < 0.0f) dt = 0.0f; // Sécurité si le temps redémarre
	last_t = t;

	// 2. Mémoire de l'épaisseur de la neige
	// On utilise un tableau statique pour se souvenir de la neige accumulée sans modifier vos structures
	static std::vector<float> snow_thickness(terrain.position.size(), 0.0f);

	// Paramètres de la simulation de neige
	float const max_snow = 0.3f;       // AVANT: 2.0f. Maintenant, c'est une fine couche.
	float const snow_rate = 0.2f;
	float const wind_speed = 0.2f;

	// Recompute the new vertices
	for (int ku = 0; ku < N; ++ku) {
		for (int kv = 0; kv < N; ++kv) {

			const float u = ku / (N - 1.0f);
			const float v = kv / (N - 1.0f);
			int const idx = ku * N + kv;

			// 3. Déplacer l'échantillonnage avec le temps (simulation de vent)
			// Ainsi, la neige ne tombe pas constamment sur les exacts mêmes sommets
			vec2 const offset = { u + t * wind_speed, v + t * wind_speed };

			// Compute the Perlin noise (on garde des paramètres de bruit fixes pour la fluidité)
			float noise = noise_perlin(offset, parameters.octave, parameters.persistency, parameters.frequency_gain);
			
			// On ne garde que les valeurs positives pour simuler des "zones de précipitations"
			float snow_fall = std::max(0.0f, noise);

			// ... (calcul de offset, noise, snow_fall) ...

			// Récupérer la pente du terrain (1.0 = plat, < 0.5 = très pentu)
			float pente = terrain.normal[idx].z;

			// Si le terrain est trop pentu (ex: falaise de montagne), la limite de neige locale est plus basse
			float limite_neige_locale = max_snow * pente;

			if (snow_thickness[idx] < limite_neige_locale) {
				float increment = snow_fall * snow_rate * dt;

				if (snow_thickness[idx] + increment > limite_neige_locale) {
					increment = limite_neige_locale - snow_thickness[idx];
				}

				snow_thickness[idx] += increment;
				terrain.position[idx].z += increment;
			}

		}
	}

	// Update the normal of the mesh structure
	terrain.normal_update();

	// Update step
	terrain_visual.vbo_position.update(terrain.position);
	terrain_visual.vbo_normal.update(terrain.normal);
}

