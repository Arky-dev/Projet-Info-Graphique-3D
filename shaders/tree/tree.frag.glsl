#version 330 core

in struct fragment_data {
    vec3 position;
    vec3 normal;
    vec3 color;
    vec2 uv;
} fragment;

layout(location=0) out vec4 FragColor;

uniform sampler2D image_texture; 
uniform sampler2D image_snow;    

uniform float snow_time;         
uniform vec3 light;

// Only declare this ONCE!
uniform float season_transition;     

void main() {
    // 1. MELTING MATH
    // Force the timer down to 0 as day approaches
    float active_snow_time = snow_time * (1.0 - season_transition);
    float snow_factor = clamp(active_snow_time / 15.0, 0.0, 1.0);

    // 2. READ TEXTURES
    vec4 color_bark = texture(image_texture, fragment.uv);
    vec4 color_snow = texture(image_snow, fragment.uv * 3.0); 

    // 3. TRUE SNOW ACCUMULATION
    vec3 n = normalize(fragment.normal);
    float up_factor = max(0.0, n.z);
    
    // As the timer goes up, the threshold to draw snow goes down.
    float snow_threshold = mix(1.1, 0.4, snow_factor);
    
    // The edge of the snow is sharp
    float final_snow_mask = smoothstep(snow_threshold - 0.05, snow_threshold + 0.05, up_factor);

    // 4. APPLY THE MASK 
    vec3 final_surface_color = mix(color_bark.rgb, color_snow.rgb, final_snow_mask);

    // 5. LIGHTING
    vec3 light_dir = normalize(light - fragment.position);
    float diffuse_intensity = max(0.0, dot(n, light_dir));
    
    vec3 ambient = vec3(0.3) * final_surface_color; 
    vec3 diffuse = vec3(0.7) * diffuse_intensity * final_surface_color; 

    FragColor = vec4(ambient + diffuse, 1.0);
}