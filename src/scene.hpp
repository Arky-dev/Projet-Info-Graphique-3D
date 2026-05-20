#pragma once


#include "cgp/cgp.hpp"

#include "environment.hpp"

#include "particles/particles.hpp"

#include "terrain.hpp"

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
	camera_controller_orbit_euler camera_control; 

	// The model of camera projection (intrinsic parameters)
	camera_projection_perspective camera_projection;

	
	
	// ****************************** //
	// Elements and shapes of the scene
	// ****************************** //

	mesh_drawable global_frame;          // The standard global frame
	particle_system_structure particle_system;     // The container of the active particles
	cgp::mesh_drawable snowflake;     // Mesh used to represent the snowflakes
	cgp::mesh_drawable billboard;   // Quadrangle used to display the billboards
	cgp::mesh_drawable sphere_light; // The moon

	void display_snowflake();
	void display_billboard();

	timer_basic timer;


	cgp::mesh terrain_mesh;
	cgp::mesh_drawable terrain_drawable;
	std::vector<float> noisy_tab;
	perlin_noise_parameters parameters;
	cgp::vec3 wind;
	mesh_drawable cloud;
	int N_instances;

	// ****************************** //
	// Callback functions
	// ****************************** //
	void mouse_move_event();
	void mouse_click_event();
	void keyboard_event();
	void idle_frame();

};





