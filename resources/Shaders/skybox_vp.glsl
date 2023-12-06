#version 330 core
// https://learnopengl.com/code_viewer_gh.php?code=src/4.advanced_opengl/6.1.cubemaps_skybox/6.1.skybox.vs

layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 view_mat;
uniform mat4 projection_mat;

void main()
{
    TexCoords = aPos;
    vec4 pos = projection_mat * view_mat * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}  