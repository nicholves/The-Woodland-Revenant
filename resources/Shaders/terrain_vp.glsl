#version 140

// Vertex buffer
in vec3 vertex;
in vec3 normal;
in vec3 color; // this is actually the tangent vector we will use texture for colour
in vec2 uv;

// Uniform (global) buffer
uniform mat4 world_mat;
uniform mat4 view_mat;
uniform mat4 projection_mat;
uniform mat4 normal_mat;

// Attributes forwarded to the fragment shader
out vec3 normal_interp;
out vec2 uv_interp;
out vec3 fragPos;


void main()
{
    vec3 tangent = color;

    fragPos = vec3(world_mat * vec4(vertex, 1.0));
    gl_Position = projection_mat * view_mat * vec4(fragPos, 1.0);

    normal_interp = mat3(transpose(inverse(world_mat))) * normal;

    uv_interp = uv;
}
