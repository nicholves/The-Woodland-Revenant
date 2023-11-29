#version 140

// Passed from the vertex shader
in vec2 uv0;

// Passed from outside
uniform float timer;
uniform sampler2D texture_map;

void main() 
{
	vec4 pixel;
	vec2 pos = uv0;
	pixel = texture(texture_map,uv0);
	
	float offset  = 0.05*(sin(timer*8.2+8.0*pos.y));
    pos.x = pos.x + offset;
	
    pixel = texture(texture_map, pos) ;
  	gl_FragColor = pixel;
}
