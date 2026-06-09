#version 330 core

in vec2 fragment_uv;
out vec4 FragColor;

uniform sampler2D image_texture; 

void main() {
    vec4 color = texture(image_texture, fragment_uv);
    
    if (color.a < 0.5) {
        discard;
    }
    
    FragColor = color;
}