#version 330 core

in struct fragment_data {
    vec3 position;
    vec3 normal;
    vec3 color;
    vec2 uv;
} fragment;

out vec4 FragColor;

uniform sampler2D image_texture; // roche
uniform sampler2D image_snow;    // neige
uniform sampler2D image_grass;   // herbe

uniform float time;
uniform vec3 light;

// Code généré à l'aide d'une IA
// --- Compact 2D GLSL Noise Functions ---
vec2 hash( vec2 p ) {
	p = vec2( dot(p,vec2(127.1,311.7)), dot(p,vec2(269.5,183.3)) );
	return -1.0 + 2.0*fract(sin(p)*43758.5453123);
}

float noise( in vec2 p ) {
    const float K1 = 0.366025404; 
    const float K2 = 0.211324865; 
	vec2  i = floor( p + (p.x+p.y)*K1 );
    vec2  a = p - i + (i.x+i.y)*K2;
    float m = step(a.y,a.x); 
    vec2  o = vec2(m,1.0-m);
    vec2  b = a - o + K2;
	vec2  c = a - 1.0 + 2.0*K2;
    vec3  h = max( 0.5-vec3(dot(a,a), dot(b,b), dot(c,c) ), 0.0 );
	vec3  n = h*h*h*h*vec3( dot(a,hash(i+0.0)), dot(b,hash(i+o)), dot(c,hash(i+1.0)));
    return dot( n, vec3(70.0) );
}
// ---------------------------------------
// Fin du code généré à l'aide d'une IA

uniform float season_transition;
uniform vec3 cloud_0;
uniform vec3 cloud_1;
uniform float terrain_snow_timer;

void main() {
    vec4 color_rock  = texture(image_texture, fragment.uv);
    vec4 color_snow  = texture(image_snow, fragment.uv);
    vec4 color_grass = texture(image_grass, fragment.uv * 4.0);
    
    color_snow.rgb = min(color_snow.rgb * 1.2, 1.0); 
    
    // Normal and Light calculations
    vec3 normal = normalize(fragment.normal);
    vec3 light_dir = normalize(light - fragment.position);
    float slope = normal.z; 

    // --- 2. DAYTIME GRASS MIX ---
    // Only grow grass on the relatively flat areas
    float flat_mask = smoothstep(0.75, 0.85, slope);
    
    // As day transitions, smoothly fade the rock into grass
    vec4 ground_color = mix(color_rock, color_grass, season_transition * flat_mask);
    // ----------------------------

    // Parameters
    float wind_speed = 0.01;
    float snow_rate = 0.05; 
    
    // 3. Procedural Snow Accumulation
    float snow_presence = 0.0;
    
    // Only accumulate on relatively flat surfaces
    if (slope > 0.75) {
        
        // --- UV WARPING ---
        vec2 terrain_warp = normal.xy * 2.0; 
        float friction_noise = noise(fragment.uv * 50.0) * 0.1;
        vec2 warped_uv = (fragment.uv * 2.5) + terrain_warp + vec2(friction_noise);

        // Apply Wind diagonally (Keep using global 'time' here so wind never stops!)
        vec2 offset = warped_uv + (time * vec2(wind_speed, wind_speed * 0.3));
        
        // --- CLUMPING ---
        float n = noise(offset) * 0.5 + 0.5; 
        n = pow(n, 2.0); 
        
        // Use terrain_snow_timer instead of time!
        float accumulation = (n + 0.3) * (terrain_snow_timer * snow_rate);
        
        // SUNLIGHT TWEAK
        float sun_exposure = max(dot(normal, light_dir), 0.0);
        accumulation -= (sun_exposure * 0.1); 
        
        // MELTING MATH
        accumulation *= (1.0 - season_transition);
        
        snow_presence = clamp(accumulation, 0.0, 1.0);
    }
    
    // 4. Smooth the edges sharply to avoid muddy grey shadows
    float blend_factor = smoothstep(0.4, 0.45, snow_presence);
    
    // THE FIX: Mix the snow OVER the combined rock/grass ground layer!
    vec4 final_color = mix(ground_color, color_snow, blend_factor);
    
    // 5. Final Basic Diffuse Lighting
    float diffuse = max(dot(normal, light_dir), 0.2); // 0.2 is ambient light

    // 6. CLOUD SHADOWS
    // Calculate distance from terrain pixel to the cloud's X/Y position
    float shadow_1 = smoothstep(3.0, 0.0, length(fragment.position.xy - cloud_0.xy));
    float shadow_2 = smoothstep(3.0, 0.0, length(fragment.position.xy - cloud_1.xy));
    
    // Combine shadows and only apply them during the day
    float total_shadow = clamp(shadow_1 + shadow_2, 0.0, 1.0) * season_transition;
    
    // Apply the shadow darkening directly to final_color
    final_color.rgb *= (1.0 - total_shadow * 0.4);
    
    FragColor = vec4(final_color.rgb * diffuse, 1.0);
}