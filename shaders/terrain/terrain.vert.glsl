#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 color;
layout (location = 3) in vec2 uv;

out struct fragment_data {
    vec3 position;
    vec3 normal;
    vec3 color;
    vec2 uv;
} fragment;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    // Standard rigid object transformation
    vec4 world_position = model * vec4(position, 1.0);
    
    // Pass data to the fragment shader
    fragment.position = world_position.xyz;
    fragment.color = color;
    fragment.uv = uv;
    
    // Correctly rotate the normals for lighting
    mat3 normal_matrix = mat3(transpose(inverse(model)));
    fragment.normal = normalize(normal_matrix * normal);

    gl_Position = projection * view * world_position;
}