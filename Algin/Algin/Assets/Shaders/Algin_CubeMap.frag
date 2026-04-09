#version 450 core
out vec4 FragColor;

in vec3 vTexCoord;

uniform samplerCube skybox;

void main()
{
    FragColor = texture(skybox, vec3(-vTexCoord.x, -vTexCoord.y, vTexCoord.z));
}