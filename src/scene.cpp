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
		{ 3.0f, -2.0f, 4.5f } /* position of the camera in the 3D scene */,
		{0,0,3} /* targeted point in 3D scene */,
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

	// Sky
	sky.initialize_data_on_gpu(mesh_primitive_sphere(50.0f));
	sky.shader.load(project::path + "shaders/sky/sky.vert.glsl", project::path + "shaders/sky/sky.frag.glsl"); // Special shader for the sky

	// Skybox
	/*
	image_structure image_skybox_template = image_load_file(project::path + "assets/skybox/skybox_snowy_2.jpg");
	// Other possibilities:
	// //   image_structure image_skybox_template = image_load_file(project::path+"assets/skybox_01.jpg");
	//   image_structure image_skybox_template = image_load_file(project::path+"assets/skybox_02.jpg");
	//   image_structure image_skybox_template = image_load_file(project::path+"assets/skybox_debug.png");


	//// Split the image into a grid of 4 x 3 sub-images
	std::vector<image_structure> image_grid = image_split_grid(image_skybox_template, 4, 3);


	skybox.initialize_data_on_gpu();
	skybox.texture.initialize_cubemap_on_gpu(image_grid[1], image_grid[7], image_grid[5], image_grid[3], image_grid[10], image_grid[4]);
	// Look at skybox_debug.png to see the correspondance of the image index
	skybox.skybox_rotation = rotation_transform::from_vector_transform({ 0,0,1 }, {0,1,0}).matrix();
	*/

	gui.display_frame = true;

	// Terrain

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

	environment.light = { 3, 3, 10 };

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
	N_instances = 50000;
	// Add a per-instance vertex attribute for the position
	for (int i = 0; i < N_instances; ++i)
	{
		particle_snowflake p = particle_snowflake(10.0f);
		particle_system.snowflakes.push_back(p);
		particle_system.snowflake_positions_and_sizes.push_back(vec4(p.p0, p.size));
	}
	// Wind
	wind = { 0.0f, 0.0f, 0.0f };
	// Send initial positions and sizes as a new buffer of data to the shader
	snowflake.initialize_supplementary_data_on_gpu(particle_system.snowflake_positions_and_sizes, /*location*/ 4, /*divisor: 1=per instance, 0=per vertex*/ 1);
	std::cout << "End function scene_structure::initialize()" << std::endl;
}

void scene_structure::display_snowflake()
{
	draw(snowflake, environment, N_instances);
}


// This function is called permanently at every new frame
// Note that you should avoid having costly computation and large allocation defined there. This function is mostly used to call the draw() functions on pre-existing data.
void scene_structure::display_frame()
{
	// Set the light to the current position of the camera
    camera_projection.aspect_ratio = window.aspect_ratio();
	environment.camera_projection = camera_projection.matrix();
	environment.camera_view = camera_control.camera_model.matrix_view();
	environment.light = { 3,3,10 };

	// Skybox
	/*
	//  Must be called before drawing the other shapes and without writing in the Depth Buffer
	glDepthMask(GL_FALSE); // disable depth-buffer writing
	draw(skybox, environment);
	glDepthMask(GL_TRUE);  // re-activate depth-buffer write
	*/

	// Sky
	sky_colors.update_color(timer.t);
	glDepthMask(GL_FALSE); // disable depth-buffer writing
	environment.uniform_generic.uniform_vec3["zenith_color"] = sky_colors.zenith_color;
	environment.uniform_generic.uniform_vec3["horizon_color"] = sky_colors.horizon_color;
	draw(sky, environment);
	glDepthMask(GL_TRUE);  // re-activate depth-buffer write

	// Light source
	sphere_light.model.translation = environment.light;
	sphere_light.material.color = gui.light_color * 0.8f;
	sphere_light.material.phong.ambient = 1;
	sphere_light.material.phong.diffuse = 0;
	sphere_light.material.phong.specular = 0;
	 draw(sphere_light, environment);

	// Draw the 3D reference frame axes if enabled
	if (gui.display_frame)
		draw(global_frame, environment);

	// Timer
	timer.update();
	environment.uniform_generic.uniform_float["time"] = timer.t;

	// Wind
	environment.uniform_generic.uniform_vec3["wind"] = wind;

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
	// Draw the instances of snowflakes
	draw(snowflake, environment, N_instances);

	if (gui.display_wireframe) {
		draw_wireframe(terrain_drawable, environment);
	}

	
	temp_terrain_update(terrain_mesh, terrain_drawable, parameters, noisy_tab, timer.t);

}


void scene_structure::display_gui()
{
	ImGui::Checkbox("Frame", &gui.display_frame);
	ImGui::Checkbox("Wireframe", &gui.display_wireframe);
	// Wind
	ImGui::SliderFloat("Wind x", &wind.x, -1.0f, 1.0f);
	ImGui::SliderFloat("Wind y", &wind.y, -1.0f, 1.0f);
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