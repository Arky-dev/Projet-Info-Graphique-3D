#include "scene.hpp"


using namespace cgp;

void deform_terrain(mesh& m)
{
	// Set the terrain to have a gaussian shape
	for (int k = 0; k < m.position.size(); ++k)
	{
		vec3& p = m.position[k];
		float d2 = p.x*p.x + p.y * p.y;
		float z = exp(-d2 / 4)-1;

		z = z + 0.05f*noise_perlin({ p.x,p.y });

		p = { p.x, p.y, z };
	}

	m.normal_update();
}


// Main initialization function called once at program startup
// Sets up the camera, 3D scene elements, and the image animation system
void scene_structure::initialize()
{
	
	std::cout << "Start function scene_structure::initialize()" << std::endl;

	// Set the behavior of the camera and its initial position
	// ********************************************** //
	camera_control.initialize(inputs, window); 
	camera_control.set_rotation_axis_z(); // camera rotates around z-axis
	//   look_at(camera_position, targeted_point, up_direction)
	camera_control.look_at(
		{ 5.0f, -4.0f, 3.5f } /* position of the camera in the 3D scene */,
		{0,0,0} /* targeted point in 3D scene */,
		{0,0,1} /* direction of the "up" vector */);

	camera_projection = camera_projection_perspective{
		50.0f * Pi/180, // Field of view
		1.0f,           // Aspect ratio
		0.01f,          // Depth min
		1000            // Depth max
	};


	// General information
	display_info();

	// Create 3D coordinate frame (x, y, z axes) for visual reference
	global_frame.initialize_data_on_gpu(mesh_primitive_frame());

	// Initialize the shapes of the scene
	// ***************************************** //

	gui.display_frame = true;

	/*float L = 5.0f;
	mesh terrain_mesh = mesh_primitive_grid({ -L,-L,0 }, { L,-L,0 }, { L,L,0 }, { -L,L,0 }, 100, 100);
	deform_terrain(terrain_mesh);
	terrain.initialize_data_on_gpu(terrain_mesh);
	terrain.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/snow.jpg");*/

	int N = 180;

	for (int ku = 0; ku < N; ++ku) {
		for (int kv = 0; kv < N; ++kv) {
			const float u = ku / (N - 1.0f);
			const float v = kv / (N - 1.0f);
			float const noise = noise_perlin({ u, v }, 4, 0.25, 1.5);
			noisy_tab.insert(noisy_tab.end(), noise);
		}
	}

	terrain_mesh = create_terrain_mesh();
	terrain_drawable.initialize_data_on_gpu(terrain_mesh);
	terrain_drawable.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/snow.jpg",
		GL_REPEAT,
		GL_REPEAT);
	update_terrain(terrain_mesh, terrain_drawable, parameters, noisy_tab, 0.0);

	// Sphere used to display the position of a light
	sphere_light.initialize_data_on_gpu(mesh_primitive_sphere(0.2f));

	environment.light = { 3, 3, 3 };

	// Remove warnings for unset uniforms
	cgp_warning::max_warning = 1;

	// Snowflakes
	// Create a quad to display snowflakes as impostors
	float l = 0.05f;
	mesh quad = mesh_primitive_quadrangle({ 0, 0,0 }, { l,0,0 }, { l,l,0 }, { 0,l,0 });
	snowflake.initialize_data_on_gpu(quad);
	snowflake.material.phong = { 1,1,1,64 };
	snowflake.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/snowflakes/1.png");
	snowflake.shader.load(project::path + "shaders/snowflake/snowflake.vert.glsl", project::path + "shaders/snowflake/snowflake.frag.glsl"); // special shader for the snowflakes
	snowflake.material.color = { 1.0f, 1.0f,1.0f };

	// Number of snowflakes displayed using Instancing (value can be higher)
	N_instances = 1000;
	// Add a per-instance vertex attribute for the position
	for (int i = 0; i < N_instances; ++i)
		particle_system.snowflakes.push_back(particle_snowflake(timer_snowflake.t));
	particle_system.evaluate_all_positions(timer.t);
	// Send these positions and sizes as a new buffer of data to the shader
	snowflake.initialize_supplementary_data_on_gpu(particle_system.snowflake_positions_and_sizes, /*location*/ 4, /*divisor: 1=per instance, 0=per vertex*/ 1);
	// timer_snowflake.event_period is the time between two spawns for the snowflakes
	timer_snowflake.event_period = 0.2f;

	std::cout << "End function scene_structure::initialize()" << std::endl;
}

void scene_structure::display_snowflake()
{
	// Evaluate the positions and display the snowflakes
	particle_system.evaluate_all_positions(timer.t);
	int N = particle_system.snowflake_positions_and_sizes.size();

	// Send these positions as a new buffer of data to the shader
	snowflake.update_supplementary_data_on_gpu(particle_system.snowflake_positions_and_sizes, /*location*/ 4, /*size elements update*/ N);
	draw(snowflake, environment, N);
}


// This function is called permanently at every new frame
// Note that you should avoid having costly computation and large allocation defined there. This function is mostly used to call the draw() functions on pre-existing data.
void scene_structure::display_frame()
{
	// Set the light to the current position of the camera
    camera_projection.aspect_ratio = window.aspect_ratio();
	environment.camera_projection = camera_projection.matrix();
	environment.camera_view = camera_control.camera_model.matrix_view();

	environment.light = { 3,3,3 };
	sphere_light.model.translation = environment.light;
	sphere_light.material.color = gui.light_color * 0.8f;
	sphere_light.material.phong.ambient = 1;
	sphere_light.material.phong.diffuse = 0;
	sphere_light.material.phong.specular = 0;
	draw(sphere_light, environment);

	// Draw the 3D reference frame axes if enabled
	if (gui.display_frame)
		draw(global_frame, environment);

	// Timers
	timer.update();
	timer_snowflake.update();

	environment.uniform_generic.uniform_float["time"] = timer.t;

	auto const& camera = camera_control.camera_model;

	// Re-orient the snow shape to always face the camera direction
	vec3 const right = camera.right();
	vec3 const up = camera.up();
	// Rotation such that the snow always faces the camera
	mat3 R = camera_control.camera_model.orientation().matrix();
	// Send rotation matrix to shader
	environment.uniform_generic.uniform_mat3["R"] = R;

	// Draw all the shapes
	draw(terrain_drawable, environment);
	draw(cube1, environment);
	// Draw the instances of snowflakes
	display_snowflake();

	if (gui.display_wireframe) {
		draw_wireframe(terrain, environment);
	}

	// Handling creation/remove of particles
	if (gui.display_wireframe) {
		draw_wireframe(terrain, environment);
		draw_wireframe(terrain_drawable, environment);
	}

	// Handling creation/remove of particles
	//if (timer_snowflake.event)
	//	particle_system.snowflakes.push_back(particle_snowflake(timer_snowflake.t));

	
	temp_terrain_update(terrain_mesh, terrain_drawable, parameters, noisy_tab, timer_snowflake.t);


	////if (gui.display_sphere) {
	////	display_snowflake();
	////}*/*/

}


void scene_structure::display_gui()
{
	ImGui::Checkbox("Frame", &gui.display_frame);
	ImGui::Checkbox("Wireframe", &gui.display_wireframe);

	ImGui::Checkbox("Display sphere", &gui.display_sphere);
}




void scene_structure::mouse_move_event()
{
	if (!inputs.keyboard.shift)
		camera_control.action_mouse_move();
	
}
void scene_structure::mouse_click_event()
{
	camera_control.action_mouse_click();
}
void scene_structure::keyboard_event()
{
	camera_control.action_keyboard();
}
void scene_structure::idle_frame()
{
	camera_control.idle_frame();
	
}

void scene_structure::display_info()
{
	std::cout << "\nCAMERA CONTROL:" << std::endl;
	std::cout << "-----------------------------------------------" << std::endl;
	std::cout << camera_control.doc_usage() << std::endl;
	std::cout << "-----------------------------------------------\n" << std::endl;


	std::cout << "\nSCENE INFO:" << std::endl;
	std::cout << "-----------------------------------------------" << std::endl;
	std::cout << "Example of scene to start a project." << std::endl;
	std::cout << "-----------------------------------------------\n" << std::endl;
}