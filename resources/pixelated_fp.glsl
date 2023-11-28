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

	// tv blockiness
	float spacing = 0.004;
	float thick = 0.002;
	if ((mod(uv0.y, spacing) < thick) || (mod(uv0.x,spacing) < thick))
		pixel = vec4(0,0,0,1);
	
  gl_FragColor = pixel;
}
