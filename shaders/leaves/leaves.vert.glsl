#version 330 core

layout (location = 0) in vec3 position;
layout (location = 3) in vec2 uv;
layout (location = 4) in vec3 instance_pos; 

uniform mat4 view;
uniform mat4 projection;
uniform float day_transition;
uniform float time;

out vec2 fragment_uv;

// Standard 3D transformation matrix
mat3 euler_to_mat3(float pitch, float yaw, float roll) {
    float cp = cos(pitch); float sp = sin(pitch);
    float cy = cos(yaw);   float sy = sin(yaw);
    float cr = cos(roll);  float sr = sin(roll);

    mat3 Rx = mat3(1.0, 0.0, 0.0,  0.0, cp, -sp,  0.0, sp, cp);
    mat3 Ry = mat3(cy, 0.0, sy,  0.0, 1.0, 0.0,  -sy, 0.0, cy);
    mat3 Rz = mat3(cr, -sr, 0.0,  sr, cr, 0.0,  0.0, 0.0, 1.0);

    return Rz * Ry * Rx;
}

uniform float leaf_fall_timer;

void main() {
    fragment_uv = uv; 

    // /!\ Ces valeurs ont été déterminées avec l'aide d'une IA générative
    float outward_angle = atan(instance_pos.y, instance_pos.x - 9.0);
    float rand_pitch = fract(sin(dot(instance_pos.xyz, vec3(12.9898, 78.233, 45.164))) * 43758.5453);
    float rand_yaw   = fract(sin(dot(instance_pos.xyz, vec3(93.9898, 67.345, 12.093))) * 43758.5453);
    float rand_roll  = fract(sin(dot(instance_pos.xyz, vec3(43.1415, 12.435, 76.908))) * 43758.5453);

    float final_pitch = rand_pitch * 6.283185f;
    float final_yaw = outward_angle - 1.57079f + ((rand_yaw - 0.5f) * 2.5f);
    float final_roll = rand_roll * 6.283115f;

    mat3 leaf_rotation = euler_to_mat3(final_pitch, final_yaw, final_roll);
    vec3 rotated_pos = leaf_rotation * position;
    // /!\ L'aide de l'IA s'arrête ici.


    float scale_factor = smoothstep(0.2, 0.8, day_transition);
    vec3 scaled_pos = rotated_pos * scale_factor;


    float t = time * 2.5;
    float phase = instance_pos.x * 0.7 + instance_pos.y * 0.7 + instance_pos.z * 0.3;
    float waveX = sin(t + phase) * 0.5 + sin(t * 1.7 + phase * 2.0) * 0.3 + sin(t * 2.3 - phase) * 0.2;
    float waveY = cos(t + phase) * 0.5 + cos(t * 1.6 - phase * 1.5) * 0.3 + cos(t * 2.1 + phase) * 0.2;
    
    float bend_factor = position.z; 
    float wind_strength = 0.07;
    scaled_pos.x += waveX * wind_strength * bend_factor * scale_factor;
    scaled_pos.y += waveY * wind_strength * bend_factor * scale_factor;


    vec3 drop_offset = vec3(0.0);
    
    if (leaf_fall_timer > 0.0) {

        // /!\ Ces valeurs ont été déterminées avec l'aide d'une IA générative
        float fall_speed = 1.5 + fract(instance_pos.x * 12.3) * 1.5;
        float fall_dist = leaf_fall_timer * fall_speed;
        drop_offset.z = -fall_dist;

        float drift_dir = (fract(instance_pos.z * 5.43) - 0.5) * 2.0; 
        drop_offset.x = drift_dir * fall_dist * 0.2; 
        drop_offset.y = (fract(instance_pos.x * 3.21) - 0.5) * fall_dist * 0.2;
        // /!\ L'aide de l'IA s'arrête ici.
    }

    vec3 world_pos = instance_pos + scaled_pos + drop_offset;

    // Collision avec le sol
    if (world_pos.z <= 0.05) {
        world_pos.z = 0.05;
        
    }

    gl_Position = projection * view * vec4(world_pos, 1.0);
}