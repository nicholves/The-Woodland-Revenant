#version 400

// Vertex buffer
in vec3 vertex;
in vec3 normal;
in vec3 color;

// Uniform (global) buffer
uniform mat4 world_mat;
uniform mat4 view_mat;
uniform mat4 normal_mat;
uniform float timer;

// Attributes forwarded to the geometry shader
out vec3 vertex_color;
out float timestep;

// Simulation parameters (constants)
uniform vec3 up_vec = vec3(0.0, 1.0, 0.0);
uniform vec3 object_color = vec3(0.8, 0.8, 0.8);
float grav = 0.0; // Gravity
float speed = 0.4; // Controls the speed of the explosion
float upward = 0.0; // additional y velocity for all particles
float distance = 10.0;

void main()
{
    // Let's first work in model space (apply only world matrix)
    vec4 position = world_mat * vec4(vertex, 1.0);
    vec4 norm = normal_mat * vec4(normal, 0.0);

    // Offset the time of each direction by the value in the normal
    vec3 t = timer*vec3(1.0, 1.0, 1.0) + normal;

    // Use the time values in different variations of sin and cos for each direction
    position.x += (cos(t.x*speed*5) * cos(t.x*speed)) * distance;
    position.y += ((sin(t.y*speed*2) + cos((t.y+1)*speed))/2) * distance;
    position.z += (cos((t.z+2)*speed*7) + sin((t.z+2.1)*speed)) * distance;
    
    // Now apply view transformation
    gl_Position = view_mat * position;
        
    // Define outputs
    // Define color of vertex
    vertex_color = vec3(1.0, 1.0, 0.0); // Yellow color 

    // Forward time step to geometry shader
    timestep = timer;
}
