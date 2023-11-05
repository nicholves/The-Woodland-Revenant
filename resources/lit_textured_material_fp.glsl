#version 130

// Attributes passed from the vertex shader
in vec3 position_interp;
in vec3 normal_interp;
in vec4 color_interp;
in vec2 uv_interp;

// Uniform (global) buffer
uniform sampler2D texture_map;
uniform vec3 light_position;
uniform vec4 light_color;
uniform float specular_power;
uniform vec3 camera_position;

void main() 
{
    // Retrieve texture value
	vec2 uv_use = uv_interp;
    vec4 pixel = texture(texture_map, uv_use);

    // Use texture in determining fragment colour

    // Lighting
    vec3 N = normalize(normal_interp);
	vec3 L = normalize(light_position - position_interp);
	vec3 R = -L + 2 * dot(L, N) * N;
	vec3 V = normalize(camera_position-position_interp);

	float diffuse = max(0.0, dot(N,L)); 
	float specular = max(0.0,dot(V,R)); 
	specular = pow(specular,specular_power); 

	float amb = 0.4; // ambient coefficient

    gl_FragColor = 
		diffuse * pixel * light_color            // Diffuse Component
		+ specular * vec4(1,1,1,1) * light_color		// Specular Component
		+ amb * pixel * light_color;             // Ambient Component
}
