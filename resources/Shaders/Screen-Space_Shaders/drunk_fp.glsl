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
	

	// drunkenness
	float offset  = 0.05*(sin(timer*0.1+8.0*pos.y));
    pos.x = pos.x + offset;
	offset  = 0.05*(cos(timer*0.1+8.0*pos.x));
    pos.y = pos.y + offset;
	pixel = texture(texture_map, pos);

	vec2 offseti;
	offseti.x = 0.4*sin(timer*2);
	offseti.y = 0.4*cos(timer);
	vec2 pos2 = uv0 + offseti;

	vec4 pixel2 = texture(texture_map,pos2);

	gl_FragColor = (pixel+pixel2)/2;
	//gl_FragColor = pixel;
}
