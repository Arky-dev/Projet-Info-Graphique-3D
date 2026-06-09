#version 330 core

in vec2 fragment_uv;
out vec4 FragColor;

uniform sampler2D image_texture; 

void main() {
    vec4 color = texture(image_texture, fragment_uv);
    
    // Discard invisible pixels so we don't draw black squares
    if (color.a < 0.5) {
        discard;
    }
    
    // THE FIX: Output the actual RGB colors from your image!
    FragColor = color;
}