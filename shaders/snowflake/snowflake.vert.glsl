#version 330 core

// Vertex shader - this code is executed for every vertex of the shape

// Inputs coming from VBOs
layout (location = 0) in vec3 vertex_position; // vertex position in local space (x,y,z)
layout (location = 1) in vec3 vertex_normal;   // vertex normal in local space   (nx,ny,nz)
layout (location = 2) in vec3 vertex_color;    // vertex color      (r,g,b)
layout (location = 3) in vec2 vertex_uv;       // vertex uv-texture (u,v)
layout (location = 4) in vec4 instance_position_and_size;  // instance position (x,y,z) and size (s)

// Output variables sent to the fragment shader
out struct fragment_data
{
    vec3 position; // vertex position in world space
    vec3 normal;   // normal position in world space
    vec3 color;    // vertex color
    vec2 uv;       // vertex uv
} fragment;

// Uniform variables expected to receive from the C++ program
uniform mat4 model; // Model affine transform matrix associated to the current shape
uniform mat4 view;  // View matrix (rigid transform) of the camera
uniform mat4 projection; // Projection (perspective or orthogonal) matrix of the camera

uniform float time; // Time in seconds since the start of the program
uniform mat3 R; // Rotation matrix such that snowflakes always face the camera
uniform vec3 wind; // Wind vector (x,y,z) applied to the snowflakes


void main()
{

	// The position of the vertex in the world space - Add the offset related to the current instance
	float size = instance_position_and_size.w;
	vec3 instance_position = instance_position_and_size.xyz;
	vec3 displacement = instance_position + size * vec3(0.0f, 0.0f, -1.0f) * time + wind * time;
	displacement.x = -10.0f + mod(displacement.x + 10.0f, 20.0f);
	displacement.y = -10.0f + mod(displacement.y + 10.0f, 20.0f);
	displacement.z = mod(displacement.z, 5.0f);
	vec3 p = R * vertex_position * size + displacement;
	
	vec4 position = model * vec4(p, 1.0f);
	
	// position = position + vec4(0.0, 0.0, - 0.5 * time, 0.0); // Add a vertical oscillation to the position of the vertex (to make the snowflake move up and down)

	// The normal of the vertex in the world space
	mat4 modelNormal = transpose(inverse(model));
	vec4 normal = modelNormal * vec4(vertex_normal, 0.0);

	// The projected position of the vertex in the normalized device coordinates:
	vec4 position_projected = projection * view * position;

	// Fill the parameters sent to the fragment shader
	fragment.position = position.xyz;
	fragment.normal   = normal.xyz;
	fragment.color = vertex_color;
	fragment.uv = vertex_uv;

	// gl_Position is a built-in variable which is the expected output of the vertex shader
	gl_Position = position_projected; // gl_Position is the projected vertex position (in normalized device coordinates)
}