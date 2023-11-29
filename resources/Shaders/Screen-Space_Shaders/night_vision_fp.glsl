#version 140

// Passed from the vertex shader
in vec2 uv0;

// Passed from outside
uniform float timer;
uniform sampler2D texture_map;
uniform float aspect_ratio;

void main() 
{
	vec4 pixel;
	vec2 pos = uv0;
	pixel = texture(texture_map,uv0);
	
	// adjust for aspect ratio
	pos.x = 2.0 * (pos.x - 0.5) * aspect_ratio + 0.5;
	pos.y = 2.0 * (pos.y - 0.5) + 0.5;


	float distanceToCenter = length(vec2(0.5, 0.5) - pos);
	if (distanceToCenter < 1.5) {
		float avgBrightness = (pixel.r + pixel.g + pixel.b) / 3;
		pixel = vec4(0, 10 * avgBrightness, 0, 1);
	}
	
  	gl_FragColor = pixel;
}
