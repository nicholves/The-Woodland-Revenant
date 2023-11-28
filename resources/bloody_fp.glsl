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
	vec2 pos = uv0;
	pixel = texture(texture_map,uv0);


	// adjust for aspect ratio
	pos.x = 2.0 * (pos.x - 0.5) * aspect_ratio + 0.5;
	pos.y = 2.0 * (pos.y - 0.5) + 0.5;

	// if (pos.x < bloodAmount || 1 - pos.x < bloodAmount || pos.y < bloodAmount || 1 - pos.y < bloodAmount) {
	// 	// the minimum distance to the border of the screen
	// 	float distance = min(min(min(pos.x, 1-pos.x), pos.y), 1-pos.y);

	// 	float bloodFactor = 1 / bloodAmount;

	// 	float newRed = 1 * (1 - distance * bloodFactor);
	// 	pixel.r = max(newRed, pixel.r);
	// 	pixel.g = pixel.g * (distance * bloodFactor);
	// 	pixel.b = pixel.b * (distance * bloodFactor);
	// }


	float distance = length(vec2(0.5, 0.5) - pos);

	float newRed = 1 * (distance * blood_factor);
	pixel.r = max(newRed, pixel.r);
	pixel.g = pixel.g * (1 - distance * blood_factor);
	pixel.b = pixel.b * (1 - distance * blood_factor);
	
  	gl_FragColor = pixel;
}
