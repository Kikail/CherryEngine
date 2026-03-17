#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 iNormal;

void main()
{
    FragColor = vec4(abs(iNormal), 1.0);
}