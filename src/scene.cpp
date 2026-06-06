#include "scene.hpp"


using namespace cgp;

float rand_interval(float min, float max) {
	return min + (max - min) * (std::rand() / (float)RAND_MAX);
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
		{ 3.0f, -2.0f, 5.5f } /* position of the camera in the 3D scene */,
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

	gui.display_frame = true;

	int N = 180;

	for (int ku = 0; ku < N; ++ku) {
		for (int kv = 0; kv < N; ++kv) {
			const float u = ku / (N - 1.0f);
			const float v = kv / (N - 1.0f);
			float const noise = noise_perlin({ u, v }, 4, 0.25, 1.5);
			noisy_tab.insert(noisy_tab.end(), noise);
		}
	}

	// Initialize ONE terrain mesh
	terrain_mesh = create_terrain_mesh();
	terrain_drawable.initialize_data_on_gpu(terrain_mesh);

	// Load Rock texture (handled automatically by terrain_drawable)
	terrain_drawable.texture.load_and_initialize_texture_2d_on_gpu(
		project::path + "assets/pierre.jpg", GL_REPEAT, GL_REPEAT);

	// Load Snow texture directly into OpenGL (we will pass this manually)
	snow_texture_id = opengl_load_texture_image(
		project::path + "assets/snow.jpg", GL_REPEAT, GL_REPEAT);

	// Apply the initial shape deformation
	update_terrain(terrain_mesh, terrain_drawable, parameters, noisy_tab, 0.0);

	// Load our new custom shader that handles the snow mathematically
	// (Assuming you have a basic standard vertex shader, you just replace the fragment shader)
	terrain_drawable.shader.load(
		project::path + "shaders/terrain/terrain.vert.glsl", // Or standard CGP terrain vert
		project::path + "shaders/terrain/terrain.frag.glsl"
	);

	// 1. Create the basic shapes (Trunk and Branches)
	mesh tree_mesh = mesh_load_file_obj(project::path + "assets/tree/raw2.obj");

	for (auto& p : tree_mesh.position) {
		p *= 1.5f; 
	}
	tree_mesh.normal_update();

	// 3. Send to GPU and set textures/shaders
	tree_drawable.initialize_data_on_gpu(tree_mesh);
	
	// Assuming you want the whole tree to use the bark texture for now
	tree_drawable.texture.load_and_initialize_texture_2d_on_gpu(
		project::path + "assets/bark.jpg", GL_REPEAT, GL_REPEAT);

	tree_drawable.shader.load(
		project::path + "shaders/tree/tree.vert.glsl", 
		project::path + "shaders/tree/tree.frag.glsl"
	);

	// 4. Position the tree in the middle of your scene
	// You can change X and Y to move it around your terrain
	tree_drawable.model.translation = { 9.f, 0.0f, 1.6f };
	tree_drawable.model.rotation = rotation_transform::from_axis_angle({ 0, 1, 0 }, -1.f* Pi / 2.0f);


	// 1. INCREASE SIZE: Change from 0.05f to 0.15f for better visibility
	float sl = 0.05f;
	cgp::mesh quad_burst = cgp::mesh_primitive_quadrangle({ -sl, -sl, 0 }, { sl, -sl, 0 }, { sl, sl, 0 }, { -sl, sl, 0 });

	tree_snow_burst.initialize_data_on_gpu(quad_burst);
	tree_snow_burst.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/snowflakes/1.png");
	tree_snow_burst.shader.load(project::path + "shaders/burst/burst.vert.glsl", project::path + "shaders/burst/burst.frag.glsl");

	// 2. SETUP INSTANCING: Create a buffer for up to 1000 particles
	glGenBuffers(1, &burst_instance_vbo);
	glBindVertexArray(tree_snow_burst.vao);
	glBindBuffer(GL_ARRAY_BUFFER, burst_instance_vbo);
	glBufferData(GL_ARRAY_BUFFER, 5000 * sizeof(cgp::vec4), nullptr, GL_DYNAMIC_DRAW);

	// Bind it to Location 4 in the shader
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(cgp::vec4), (void*)0);
	glVertexAttribDivisor(4, 1); // The magic line that tells OpenGL it is instanced!
	glBindVertexArray(0);


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
	N_instances = 10000;
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

	


	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, snow_texture_id);

	// 2. Tell the shader that "image_snow" corresponds to Texture Unit 1
	environment.uniform_generic.uniform_int["image_snow"] = 1;

	// 3. Draw the terrain (the shader calculates the snow on the fly!)
	draw(terrain_drawable, environment);

	if (gui.display_wireframe) {
		draw_wireframe(terrain_drawable, environment);
	}


	static float last_time = 0.0f;
	float dt = timer.t - last_time;
	last_time = timer.t;

	// State machine for snow and shaking
	static float tree_snow_timer = 0.0f;
	static float shake_timer = 0.0f;
	static bool is_shaking = false;

	float current_intensity = 0.0f;

	if (!is_shaking) {
		tree_snow_timer += dt;
		if (tree_snow_timer >= 10.0f) {
			is_shaking = true;
			shake_timer = 0.0f;

			// --- ADD THIS: SPAWN THE BURST! ---
			// --- SPAWN THE BURST! ---
			tree_particles.clear();

			// 1. INCREASE COUNT: Change this to 1500 or 2000 for a massive blizzard!
			for (int i = 0; i < 1500; ++i) {
				burst_particle p;

				// 1. Generate a random angle (0 to 2*Pi radians)
				float angle = rand_interval(0.0f, 6.283185f);

				// 2. Generate a random radius (0 to 3.0)
				// We use std::sqrt to ensure an even, uniform spread across the circle!
				float radius = 3.0f * std::sqrt(rand_interval(0.0f, 1.0f));

				// 3. Convert polar coordinates to Cartesian (X, Y)
				float rx = radius * std::cos(angle);
				float ry = radius * std::sin(angle);

				// Z remains a vertical spread (height of the canopy)
				float rz = rand_interval(-1.0f, 1.0f);

				p.position = { 0.0f + rx, 0.0f + ry, 1.6f + rz };

				// Gentle horizontal drift, tiny Z-axis spread
				p.velocity = { rx * 0.5f, ry * 0.5f, rand_interval(-0.2f, 0.2f) };
				p.age = rand_interval(1.0f, 2.5f);

				tree_particles.push_back(p);

			}
		}
	}
	else {
		shake_timer += dt;

		// Melt the snow visually while shaking
		tree_snow_timer = std::max(0.0f, tree_snow_timer - (dt * 10.0f));

		// ALLOW 3 SECONDS FOR THE CALM DOWN TO FINISH
		if (shake_timer >= 3.0f) {
			is_shaking = false;
			tree_snow_timer = 0.0f;
			shake_timer = 0.0f; // Reset timer when done
		}
	}

	// Prepare the shader
	glUseProgram(tree_drawable.shader.id);

	// Pass the exact shake_timer to the shader instead of an intensity toggle!
	environment.uniform_generic.uniform_float["shake_time"] = shake_timer;
	environment.uniform_generic.uniform_float["snow_time"] = tree_snow_timer;

	environment.uniform_generic.uniform_vec2["tree_pivot"] = tree_pivot;

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, snow_texture_id);
	environment.uniform_generic.uniform_int["image_snow"] = 1;

	// Draw the tree
	draw(tree_drawable, environment);

	// --- NEW PHYSICS & INSTANCED RENDERING LOOP ---
	std::vector<cgp::vec4> burst_instance_data;

	for (auto it = tree_particles.begin(); it != tree_particles.end(); ) {
		// Kinematics
		it->velocity.z -= 4.0f * dt;
		it->position += it->velocity * dt;
		it->age -= dt;

		// Kill particles that hit the floor or get too old
		if (it->age <= 0.0f || it->position.z <= 0.0f) {
			it = tree_particles.erase(it);
		}
		else {
			// Shrink the snowflake as it hits its last 0.5 seconds of life
			float current_scale = (it->age < 0.5f) ? (it->age * 2.0f) : 1.0f;

			// Pack the X, Y, Z, and Scale into a vec4 and add it to our list
			burst_instance_data.push_back({ it->position.x, it->position.y, it->position.z, current_scale });
			++it;
		}
	}

	// If we have particles alive, send them to the GPU and draw them!
	if (!burst_instance_data.empty()) {
		glBindBuffer(GL_ARRAY_BUFFER, burst_instance_vbo);

		// 1. Upload the array to the GPU
		glBufferSubData(GL_ARRAY_BUFFER, 0, burst_instance_data.size() * sizeof(cgp::vec4), burst_instance_data.data());

		// 2. Draw ALL particles at once using the instance count!
		draw(tree_snow_burst, environment, burst_instance_data.size());
	}

}


void scene_structure::display_gui()
{
	ImGui::Checkbox("Frame", &gui.display_frame);
	ImGui::Checkbox("Wireframe", &gui.display_wireframe);
	// Wind
	ImGui::SliderFloat("Wind x", &wind.x, -1.0f, 1.0f);
	ImGui::SliderFloat("Wind y", &wind.y, -1.0f, 1.0f);

	// ADD THIS SLIDER:
	ImGui::SliderFloat2("Tree Pivot (Y, Z)", &tree_pivot.x, -10.0f, 10.0f);
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