#version 330 core

in vec2 fragment_uv;
out vec4 FragColor;

uniform sampler2D image_texture; 

void main() {
    // Read the texture
    vec4 color = texture(image_texture, fragment_uv);
    
    // 1. MATCH THE THRESHOLD
    // Cut off the linearly interpolated grey/black edges by matching
    // the 0.5 threshold from your main snowflake shader.
    if (color.a < 0.5) {
        discard;
    }
    
    // 2. FORCE PURE WHITE
    // Instead of using the texture's RGB (which might contain grey edges),
    // we force it to be bright white and only use the texture's alpha channel!
    FragColor = vec4(1.0, 1.0, 1.0, color.a);
}