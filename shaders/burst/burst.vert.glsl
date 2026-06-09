#version 330 core
layout (location = 0) in vec3 position;
layout (location = 3) in vec2 uv;
// NEW: The instance data (x, y, z, scale) is fed directly into location 4!
layout (location = 4) in vec4 instance_pos_size; 

uniform mat4 view;
uniform mat4 projection;
uniform mat3 R; // Camera billboarding

out vec2 fragment_uv;

void main() {
    fragment_uv = uv;

    // Apply billboarding rotation, multiply by the instance's size, 
    // and finally add the instance's true world position.
    vec3 world_pos = instance_pos_size.xyz + R * (position * instance_pos_size.w);

    gl_Position = projection * view * vec4(world_pos, 1.0);
}