#pragma once


#include "cgp/cgp.hpp"

#include "environment.hpp"

#include "particles/particles.hpp"

#include "terrain.hpp"

#include "sky/sky.hpp"

using cgp::mesh_drawable;



struct gui_parameters {
	bool display_frame = false;
	bool display_wireframe = false;

	vec3 light_color = { 1,1,1 };
};



// The structure of the custom scene
struct scene_structure : cgp::scene_inputs_generic {
	
	// ****************************** //
	// Standard Functions
	// ****************************** //

	void initialize();  // Standard initialization to be called before the animation loop
	void display_frame();     // The frame display to be called within the animation loop
	void display_gui(); // The display of the GUI, also called within the animation loop

	// ****************************** //
	// Context
	// ****************************** //

	// Environment controller (background color, )
	environment_structure environment; 
	// Window where the scene is displayed
	window_structure window; 
	// Storage for inputs status (mouse, keyboard, window dimension)
	input_devices inputs; 
	// Standard GUI element storage
	gui_parameters gui; 

	// Display information at the start of the program
	void display_info();

	// ****************************** //
	// Camera controller
	// ****************************** //

	// Controller of the camera (extrinsic parameters: position/orientation) -- to be adapted to the desired model and behavior
	//camera_controller_orbit_euler camera_control; 

	// Special camera mode adapted to a 2D displacement mode
	camera_controller_2d_displacement camera_control;

	// The model of camera projection (intrinsic parameters)
	camera_projection_perspective camera_projection;

	
	
	// ****************************** //
	// Elements and shapes of the scene
	// ****************************** //

	mesh_drawable global_frame;          // The standard global frame
	//skybox_drawable skybox; // Skybox used to represent the background
	cgp::mesh_drawable sky; // Quad used to represent the sky (with a gradient color)
	particle_system_structure particle_system;     // The container of the active particles
	cgp::mesh_drawable snowflake;     // Mesh used to represent the snowflakes
	GLuint snowflake_1_texture_id; // Different snowflake textures
	GLuint snowflake_2_texture_id;
	GLuint snowflake_3_texture_id;
	cgp::mesh_drawable billboard;   // Quadrangle used to display the billboards
	cgp::mesh_drawable sphere_light; // The moon and sun

	sky_color sky_colors; // The color of the sky (zenith and horizon) depending on the time of the day
	light_source light_att; // The position and color of the light source depending on the time of the day

	timer_basic timer;


	cgp::mesh terrain_mesh;
	cgp::mesh snow_mesh;
	cgp::mesh_drawable terrain_drawable;
	cgp::mesh_drawable snow_drawable;
	std::vector<float> noisy_tab;
	perlin_noise_parameters parameters;
	cgp::vec3 wind;
	mesh_drawable cloud;
	int N_instances;
	GLuint snow_texture_id;
	GLuint grass_texture_id;
	cgp::mesh_drawable tree_drawable;
	cgp::vec2 tree_pivot = { 0.0f, 9.0f };

	// 1. The physics structure for the falling snow
	struct burst_particle {
		cgp::vec3 position;
		cgp::vec3 velocity;
		float age;
	};

	std::vector<burst_particle> tree_particles;
	cgp::mesh_drawable tree_snow_burst;

	// ADD THIS: The buffer that will hold our instancing data
	GLuint burst_instance_vbo = 0;

	float tree_snow_timer = 0.0f;

	// 2. State Machine Variables
	enum TreeState { IDLE, SHAKING };
	TreeState current_tree_state = IDLE;

	float tree_shake_timer = 0.0f;      // Tracks how long the tree has been shaking
	float const SNOW_MAX_TIME = 15.0f;  // Shakes after 15 seconds of snow
	float const SHAKE_DURATION = 2.0f;  // Shakes for 2 seconds

	// --- SUMMER / WINTER STATE ---
	bool is_summer = false;
	float season_transition = 0.0f; // 0.0 = Winter, 1.0 = Summer

	// --- DAY / NIGHT STATE ---
	bool is_day = false;
	float day_transition = 0.0f; // 0.0 = Night, 1.0 = Day

	// --- CLOUDS ---
	cgp::mesh_drawable cloud_drawable;
	std::vector<cgp::vec3> cloud_positions;

	// --- FLORA INSTANCING ---
	cgp::mesh_drawable grass_drawable;
	cgp::mesh_drawable leaf_drawable;

	std::vector<cgp::vec3> grass_positions;
	std::vector<cgp::vec3> leaf_positions;

	GLuint grass_vbo = 0;
	GLuint leaf_vbo = 0;

	// ****************************** //
	// Callback functions
	// ****************************** //
	void mouse_move_event();
	void mouse_click_event();
	void keyboard_event();
	void idle_frame();

};





