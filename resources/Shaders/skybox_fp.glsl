#version 330 core
// https://learnopengl.com/code_viewer_gh.php?code=src/4.advanced_opengl/6.1.cubemaps_skybox/6.1.skybox.fs

out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube texture_map;

void main()
{    
    FragColor = texture(texture_map, TexCoords);
    //FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}