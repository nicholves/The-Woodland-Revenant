#version 400

// Definition of the geometry shader
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

// Attributes passed from the vertex shader
in vec4 particle_color[];
in float particle_id[];

// Uniform (global) buffer
uniform mat4 projection_mat;

// Simulation parameters (constants)
float particle_size = 2;

// Attributes passed to the fragment shader
out vec4 frag_color;
out vec2 tex_coord;


void main(void){

    // Get the position of the particle
    vec4 position = gl_in[0].gl_Position;

    // Define the positions of the four vertices that will form a quad 
    // The positions are based on the position of the particle and its size
    // We simply add offsets to the position (we can think of it as the center of the particle),
    // since we are already in camera space / view space
    vec4 v[4];
    v[0] = vec4(position.x - 0.5*particle_size, position.y - 0.5*particle_size, position.z, 1.0);
    v[1] = vec4(position.x + 0.5*particle_size, position.y - 0.5*particle_size, position.z, 1.0);
    v[2] = vec4(position.x - 0.5*particle_size, position.y + 0.5*particle_size, position.z, 1.0);
    v[3] = vec4(position.x + 0.5*particle_size, position.y + 0.5*particle_size, position.z, 1.0);

    // Create the new geometry: a quad with four vertices from the vector v
    gl_Position = projection_mat * v[0];
    tex_coord = vec2(0.0, 0.0);
    frag_color = vec4(vec3(0.0), particle_color[0].a);
    EmitVertex();

    gl_Position = projection_mat * v[1];
    tex_coord = vec2(1.0, 0.0);
    frag_color = vec4(vec3(0.0), particle_color[0].a);
    EmitVertex();

    gl_Position = projection_mat * v[2];
    tex_coord = vec2(0.0, 1.0);
    frag_color = vec4(vec3(0.0), particle_color[0].a);
    EmitVertex();

    gl_Position = projection_mat * v[3];
    tex_coord = vec2(1.0, 1.0);
    frag_color = vec4(vec3(0.0), particle_color[0].a);
    EmitVertex();

     EndPrimitive();
}
