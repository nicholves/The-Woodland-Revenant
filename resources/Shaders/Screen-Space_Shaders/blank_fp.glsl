#version 140

// Passed from the vertex shader
in vec2 uv0;

// Passed from outside
uniform float timer;
uniform sampler2D texture_map;
uniform float aspect_ratio;
uniform float blood_factor;

void main() 
{
	vec4 pixel;
	pixel = texture(texture_map,uv0);
	
  	gl_FragColor = pixel;
}
