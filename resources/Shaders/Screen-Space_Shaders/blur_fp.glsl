#version 140

// Passed from the vertex shader
in vec2 uv0;

// Passed from outside
uniform float timer;
uniform int num_samples;
uniform sampler2D texture_map;

void main() 
{
	int n = num_samples;
	vec2 sampleSize = 1 / vec2(1920, 1080);

	vec4 vsum = vec4(0,0,0,0);
	float horizDistance = sampleSize.x;
	float vertDistance = sampleSize.y;
	float sampleWeight = 1.0 / num_samples;

	float startIndex = uv0.y - ((n / 2) * vertDistance);
	float endIndex = uv0.y + ((n / 2) * vertDistance);
	for (float i = startIndex; i < endIndex; i += vertDistance) {
		vsum += texture(texture_map, vec2(uv0.x, i)) * sampleWeight;
	}

	vec4 hsum = vec4(0,0,0,0);
	startIndex = uv0.x - ((n / 2) * horizDistance);
	endIndex = uv0.x + ((n / 2) * horizDistance);
	for (float j = startIndex; j < endIndex; j += horizDistance) {
			hsum += texture(texture_map, vec2(j, uv0.y)) * sampleWeight;
		}

	vec4 sum = (vsum + hsum) / 2;
	sum.a = 1;
	gl_FragColor = sum;
}
