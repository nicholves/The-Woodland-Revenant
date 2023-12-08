#version 330 core

// Vertex buffer
in vec3 vertex;
in vec3 normal;
in vec3 color;
in vec2 uv;
in mat4 instanceMatrix;

// Uniform (global) buffer
uniform mat4 view_mat;
uniform mat4 projection_mat;

// Attributes forwarded to the fragment shader
out vec3 normal_interp;
out vec4 color_interp;
out vec2 uv_interp;
out vec3 fragPos;


void main()
{
    fragPos = vec3(instanceMatrix * vec4(vertex, 1.0));
    gl_Position = projection_mat * view_mat * vec4(fragPos, 1.0);
    
    normal_interp = mat3(transpose(inverse(instanceMatrix))) * normal;

    color_interp = vec4(color, 1.0);

    uv_interp = uv;
}
