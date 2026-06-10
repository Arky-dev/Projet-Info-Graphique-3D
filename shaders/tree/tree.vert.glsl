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

uniform vec2 tree_pivot; 
uniform float shake_time; 

uniform float snow_time; 

uniform float season_transition;

void main() {    
    vec2 centered_yz = position.yz - tree_pivot;
    float r_dist = length(centered_yz); 
    
    float root_x = -1.0; 
    float h_dist = max(0.0, position.x - root_x); 
    
    // secousse
    float envelope = 1.0 - smoothstep(0.8, 2.5, shake_time);
    if (shake_time <= 0.0) envelope = 0.0;
    
    float inertia_delay = (r_dist * 1.5) + (h_dist * 0.2);
    float root_anchor = smoothstep(0.0, 0.001, h_dist);
    
    float branch_shake = log(r_dist + 1.0) * 0.4; 
    float raw_amplitude = 0.1 + branch_shake + (h_dist * 0.05);
    float amplitude = raw_amplitude * root_anchor; 
    
    float speed = 20.0; 
    float theta = sin(shake_time * speed - inertia_delay) * envelope * amplitude;
    
    float c = cos(theta);
    float s = sin(theta);
    
    vec3 displaced_pos = position;
    displaced_pos.y = centered_yz.x * c - centered_yz.y * s + tree_pivot.x;
    displaced_pos.z = centered_yz.x * s + centered_yz.y * c + tree_pivot.y;
    
    vec3 displaced_normal = normal;
    displaced_normal.y = normal.y * c - normal.z * s;
    displaced_normal.z = normal.y * s + normal.z * c;

    vec4 world_position = model * vec4(displaced_pos, 1.0);
    
    mat3 normal_matrix = mat3(transpose(inverse(model)));
    vec3 world_normal = normalize(normal_matrix * displaced_normal);

    // accumulation de la neige
    float active_snow_time = snow_time * (1.0 - season_transition);
    float snow_factor = clamp(active_snow_time / 15.0, 0.0, 1.0); 
    
    float up_factor = max(0.0, world_normal.z);
    float max_snow_thickness = 0.15; 
    world_position.xyz += world_normal * up_factor * snow_factor * max_snow_thickness;


    fragment.position = world_position.xyz;
    fragment.color = color;
    fragment.uv = uv;
    fragment.normal = world_normal; 

    gl_Position = projection * view * world_position;
}