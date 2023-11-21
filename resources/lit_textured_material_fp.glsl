#version 140

// Attributes passed from the vertex shader
in vec3 fragPos;
in vec3 position_interp;
in vec3 normal_interp;
in vec4 color_interp;
in vec2 uv_interp;

// Uniform (global) buffer
uniform sampler2D texture_map;
uniform vec4 light_color;
uniform vec4 ambient_light_color;
uniform float specular_power;
uniform vec3 camera_position;
uniform vec3 flashlight_pos;
uniform vec3 flashlight_dir;
uniform float cutoff;
uniform float falloffRate;

void main() 
{
    // Retrieve texture value
	vec2 uv_use = uv_interp;
    vec4 pixel = texture(texture_map, uv_use);

    // Use texture in determining fragment colour

    // Lighting
    vec3 N = normalize(normal_interp);
	vec3 L = normalize(flashlight_pos - position_interp);
	vec3 R = -L + 2 * dot(L, N) * N;
	vec3 V = normalize(camera_position-position_interp);

	// whether the frag should be iluminated
	vec3 frag_relative_to_flashlight = normalize(flashlight_pos - fragPos);
	float cosTheta = dot(frag_relative_to_flashlight, -flashlight_dir);

	float diffuse = max(0.0, dot(N,L)); 
	float specular = max(0.0,dot(V,R)); 
	specular = pow(specular,specular_power); 

	if (cosTheta < cutoff) {
		specular = 0;
		diffuse = 0;
	}

	specular = falloffRate * specular * ((cosTheta - cutoff) / (1-cutoff) );
	diffuse = falloffRate * diffuse * ((cosTheta - cutoff) / (1-cutoff) );

	float amb = 0.1; // ambient coefficient

    gl_FragColor = 
		diffuse * pixel * light_color            // Diffuse Component
		+ specular * vec4(1,1,1,1) * light_color		// Specular Component
		+ amb * pixel * ambient_light_color;             // Ambient Component
}
