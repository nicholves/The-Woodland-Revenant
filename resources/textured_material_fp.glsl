#version 130

// Attributes passed from the vertex shader
in vec3 position_interp;
in vec3 normal_interp;
in vec4 color_interp;
in vec2 uv_interp;
in vec3 light_pos;

// Uniform (global) buffer
uniform sampler2D texture_map;


void main() 
{
    // Retrieve texture value
	vec2 uv_use = uv_interp;
    vec4 pixel = texture(texture_map, uv_use);

    // Use texture in determining fragment colour

    gl_FragColor = pixel;
}
