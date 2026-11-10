#version 330 core
in vec3 TexCoords;
out vec4 color;

uniform samplerCube skybox;
uniform vec3 skyTint = vec3(1.0);   // (1,1,1) = imagen sin cambios; menor = cielo mas oscuro / tenido

void main()
{
    color = vec4(texture(skybox, TexCoords).rgb * skyTint, 1.0);
}