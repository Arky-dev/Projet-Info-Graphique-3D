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

	// Initialize ONE terrain mesh
	terrain_mesh = create_terrain_mesh();
	terrain_drawable.initialize_data_on_gpu(terrain_mesh);

	// Load Rock texture (handled automatically by terrain_drawable)
	terrain_drawable.texture.load_and_initialize_texture_2d_on_gpu(
		project::path + "assets/pierre.jpg", GL_REPEAT, GL_REPEAT);

	// Load Grass texture for the terrain
	grass_texture_id = opengl_load_texture_image(
		project::path + "assets/grass.jpg", GL_REPEAT, GL_REPEAT);

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

	environment.light = light_att.position;

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

	// Initialize Clouds
	cgp::mesh cloud_mesh = cgp::mesh_primitive_quadrangle(
		{ -1.5f,-1.5f,0 }, { 1.5f,-1.5f,0 }, { 1.5f,1.5f,0 }, { -1.5f,1.5f,0 }
	);
	cloud_drawable.initialize_data_on_gpu(cloud_mesh);

	// Create two clouds high up in the sky
	cloud_positions.push_back({ -3.0f, 2.0f, 8.0f });
	cloud_positions.push_back({ 4.0f, -1.0f, 7.5f });

	// Number of snowflakes displayed using Instancing (value can be higher)
	N_instances = 50000;
	// Add a per-instance vertex attribute for the position
	for (int i = 0; i < N_instances; ++i)
	{
		particle_snowflake p = particle_snowflake(10.0f);

		p.p0.z += 5.0f;

		particle_system.snowflakes.push_back(p);
		particle_system.snowflake_positions_and_sizes.push_back(vec4(p.p0, p.size));
	}
	// Wind
	wind = { 0.2f, 0.1f, 0.0f };
	// Send initial positions and sizes as a new buffer of data to the shader
	snowflake.initialize_supplementary_data_on_gpu(particle_system.snowflake_positions_and_sizes, /*location*/ 4, /*divisor: 1=per instance, 0=per vertex*/ 1);

	// 1. GENERATE GRASS POSITIONS (Flat, Center of Scene)
	// We iterate through your actual terrain mesh to find the perfect spots!
	for (size_t i = 0; i < terrain_mesh.position.size(); ++i) {
		cgp::vec3 p = terrain_mesh.position[i];
		cgp::vec3 n = terrain_mesh.normal[i];

		// If it's within a radius of 6.0 from the center, and flat (Normal Z > 0.85)
		if (p.x * p.x + p.y * p.y < 36.0f && n.z > 0.85f) {
			// Plant 3 clusters of grass around this spot for density
			for (int j = 0; j < 3; ++j) {
				float rx = rand_interval(-0.4f, 0.4f);
				float ry = rand_interval(-0.4f, 0.4f);
				grass_positions.push_back({ p.x + rx, p.y + ry, p.z - 0.05f }); // Push slightly into ground
			}
		}
	}

	// 2. GENERATE LEAF POSITIONS (Tree Canopy)
	// --- FIX: CANOPY-ALIGNED LEAF PLACEMENT ---
	// --- FIX: ROTATED CANOPY LEAF PLACEMENT ---
	// --- FIX: ROTATED CANOPY LEAF PLACEMENT ---
	// --- FIX: VOLUMETRIC CANOPY PLACEMENT ---
	// --- FIX: EXACT LEAF PLACEMENT ---
	leaf_positions.clear();

	// Create the exact same rotation matrix you used for the tree (-Pi/2 on Y)
	cgp::mat3 R_tree = cgp::rotation_transform::from_axis_angle({ 0, 1, 0 }, -1.0f * cgp::Pi / 2.0f).matrix();

	for (size_t i = 0; i < tree_mesh.position.size(); i += 15) {

		// 1. Grab the point (These points are ALREADY scaled by 1.5 natively!)
		cgp::vec3 p = tree_mesh.position[i];

		// 2. Measure local dimensions to filter out the trunk
		float local_height = p.x;
		float local_radius = std::sqrt(p.y * p.y + p.z * p.z);

		// 3. FILTER: Only place a leaf on the outer canopy
		// Radius > 0.8 ensures we don't put leaves inside the wooden trunk
		// Height > 2.0 ensures we don't put leaves near the roots
		if (local_radius > 0.8f && local_height > 0.0f) {

			// 4. Apply the exact Rotation and Translation of the tree
			cgp::vec3 world_leaf_pos = R_tree * p;    // Rotate first

			world_leaf_pos.x += 9.0f;                 // Translate second
			world_leaf_pos.y += 0.0f;
			world_leaf_pos.z += 1.6f;

			// Add a tiny random offset so the leaves aren't glued perfectly flat to the bark
			world_leaf_pos.x += rand_interval(-0.2f, 0.2f);
			world_leaf_pos.y += rand_interval(-0.2f, 0.2f);
			world_leaf_pos.z += rand_interval(-0.2f, 0.2f);

			leaf_positions.push_back(world_leaf_pos);
		}
	}

	// 3. CREATE THE SHAPE (Stand-up Quad, anchored at Z=0)
	float w = 0.15f;
	float h = 0.3f;
	cgp::mesh quad_flora = cgp::mesh_primitive_quadrangle({ -w, 0, 0 }, { w, 0, 0 }, { w, 0, h }, { -w, 0, h });

	// Initialize Grass
	grass_drawable.initialize_data_on_gpu(quad_flora);
	// ** IMPORTANT: Provide your own grass/leaf textures here! **
	// If you don't have images yet, you can comment the texture lines out and the shader will draw them white/grey.
	grass_drawable.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/grass.jpg");
	grass_drawable.shader.load(project::path + "shaders/leaves/leaves.vert.glsl", project::path + "shaders/leaves/leaves.frag.glsl");

	// Initialize Leaves (Reuse the exact same quad and shader!)
	leaf_drawable.initialize_data_on_gpu(quad_flora);
	leaf_drawable.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/leaf.png");
	leaf_drawable.shader.load(project::path + "shaders/leaves/leaves.vert.glsl", project::path + "shaders/leaves/leaves.frag.glsl");

	// 4. BIND VBOs (Send the positions to the GPU permanently)
	// Grass Buffer
	glGenBuffers(1, &grass_vbo);
	glBindVertexArray(grass_drawable.vao);
	glBindBuffer(GL_ARRAY_BUFFER, grass_vbo);
	glBufferData(GL_ARRAY_BUFFER, grass_positions.size() * sizeof(cgp::vec3), grass_positions.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(cgp::vec3), (void*)0);
	glVertexAttribDivisor(4, 1);

	// Leaf Buffer
	glGenBuffers(1, &leaf_vbo);
	glBindVertexArray(leaf_drawable.vao);
	glBindBuffer(GL_ARRAY_BUFFER, leaf_vbo);
	glBufferData(GL_ARRAY_BUFFER, leaf_positions.size() * sizeof(cgp::vec3), leaf_positions.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(cgp::vec3), (void*)0);
	glVertexAttribDivisor(4, 1);

	glBindVertexArray(0);

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

	static float last_time = 0.0f;
	float dt = timer.t - last_time;
	last_time = timer.t;

	// Sky
	if (is_day) {
		day_transition = std::min(1.0f, day_transition + dt * 0.25f);
	}
	else {
		day_transition = std::max(0.0f, day_transition - dt * 0.25f);
	}
	sky_colors.update_color(day_transition);
	environment.uniform_generic.uniform_vec3["zenith_color"] = sky_colors.zenith_color;
	environment.uniform_generic.uniform_vec3["horizon_color"] = sky_colors.horizon_color;

	// Light source
	light_att.update_light(day_transition);
	sphere_light.model.translation = light_att.position;
	sphere_light.material.color = light_att.color;
	sphere_light.material.phong.ambient = 1;
	sphere_light.material.phong.diffuse = 0;
	sphere_light.material.phong.specular = 0;
	environment.light = light_att.position;

	glDepthMask(GL_FALSE); // disable depth-buffer writing
	draw(sky, environment);
	draw(sphere_light, environment);
	glDepthMask(GL_TRUE);  // re-activate depth-buffer write



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
	if (season_transition < 1.0f) {
		glUseProgram(snowflake.shader.id);
		environment.uniform_generic.uniform_float["season_transition"] = season_transition;
		draw(snowflake, environment, N_instances);
	}


	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, snow_texture_id);

	// ADD THIS FOR THE GRASS
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, grass_texture_id);
	environment.uniform_generic.uniform_int["image_grass"] = 2;

	// Draw the terrain...

	// 2. Tell the shader that "image_snow" corresponds to Texture Unit 1
	environment.uniform_generic.uniform_int["image_snow"] = 1;

	// 3. Draw the terrain (the shader calculates the snow on the fly!)
	draw(terrain_drawable, environment);

	if (gui.display_wireframe) {
		draw_wireframe(terrain_drawable, environment);
	}

	// State machine for snow and shaking
	// State machine for snow and shaking
	static float tree_snow_timer = 0.0f;
	static float terrain_snow_timer = 0.0f; // NEW: Dedicated timer for ground snow
	static float shake_timer = 0.0f;
	static bool is_shaking = false;

	// NEW: Track the falling leaves!
	static float leaf_fall_timer = 0.0f;

	if (is_summer) {
		tree_snow_timer = std::max(0.0f, tree_snow_timer - (dt * 5.0f));
		terrain_snow_timer = std::max(0.0f, terrain_snow_timer - (dt * 5.0f));
		is_shaking = false;
		shake_timer = 0.0f;

		// Reset the leaves instantly so they bloom on the branches, not the ground!
		leaf_fall_timer = 0.0f;
	}
	else {
		terrain_snow_timer += dt;

		// As the sun sets, start the falling animation
		if (season_transition > 0.0f) {
			leaf_fall_timer = std::min(leaf_fall_timer + dt, 10.0f); // Cap at 10s
		}

		if (!is_shaking) {
			tree_snow_timer += dt;
			if (tree_snow_timer >= 10.0f) {
				is_shaking = true;
				shake_timer = 0.0f;

				// --- SPAWN THE BURST HERE ---
				tree_particles.clear();
				for (int i = 0; i < 1500; ++i) {
					burst_particle p;
					float angle = rand_interval(0.0f, 6.283185f);
					float radius = 3.0f * std::sqrt(rand_interval(0.0f, 1.0f));
					float rx = radius * std::cos(angle);
					float ry = radius * std::sin(angle);
					float rz = rand_interval(2.0f, 6.0f);
					p.position = { rx, 0.0f + ry, -1.6f + rz };
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

			if (shake_timer >= 3.0f) {
				is_shaking = false;
				tree_snow_timer = 0.0f;
				shake_timer = 0.0f;
			}
		}
	}

	// Prepare the shaders and pass ALL timers
	glUseProgram(tree_drawable.shader.id);
	environment.uniform_generic.uniform_float["shake_time"] = shake_timer;
	environment.uniform_generic.uniform_float["snow_time"] = tree_snow_timer;

	// Pass the new terrain timer to the environment so the terrain shader can read it!
	// Pass the new terrain timer to the environment so the terrain shader can read it!
	environment.uniform_generic.uniform_float["terrain_snow_timer"] = terrain_snow_timer;

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

	// 1. SMOOTH TRANSITION
	// It takes ~3 seconds to fully transition between day and night
	if (is_summer) {
		season_transition = std::min(1.0f, season_transition + dt * 0.3f);
	}
	else {
		season_transition = std::max(0.0f, season_transition - dt * 0.3f);
	}


	// 3. MOVE CLOUDS (Wind)
	for (auto& pos : cloud_positions) {
		pos.x += dt * 0.5f;
		if (pos.x > 15.0f) pos.x = -15.0f; // Wrap around the sky
	}

	// 4. SEND MASTER STATE TO ALL SHADERS
	environment.uniform_generic.uniform_float["season_transition"] = season_transition;
	environment.uniform_generic.uniform_vec3["cloud_0"] = cloud_positions[0];
	environment.uniform_generic.uniform_vec3["cloud_1"] = cloud_positions[1];

	// --- DRAW FLORA ---
	// Only bother drawing if they are large enough to see!
	// --- DRAW FLORA ---
	// --- DRAW FLORA ---
	if (season_transition > 0.45f) {
		// Draw Leaves ONLY! (The terrain shader handles the grass texture automatically)
		glUseProgram(leaf_drawable.shader.id);

		// Notice we removed the "R" matrix here, we don't want them to billboard anymore!
		environment.uniform_generic.uniform_float["season_transition"] = season_transition;
		environment.uniform_generic.uniform_float["time"] = timer.t;

		environment.uniform_generic.uniform_float["leaf_fall_timer"] = leaf_fall_timer;
		draw(leaf_drawable, environment, leaf_positions.size());
	}

}


void scene_structure::display_gui()
{
	ImGui::Checkbox("Frame", &gui.display_frame);
	ImGui::Checkbox("Wireframe", &gui.display_wireframe);
	// Wind
	ImGui::SliderFloat("Wind x", &wind.x, -1.0f, 1.0f);
	ImGui::SliderFloat("Wind y", &wind.y, -1.0f, 1.0f);

	ImGui::Checkbox("Toggle Winter / Summer", &is_summer);
	ImGui::Checkbox("Toggle Night / Day", &is_day);
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