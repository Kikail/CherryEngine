#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 iNormal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // Position dans l'espace "Monde" (utile pour la lumière)
    FragPos = vec3(model * vec4(aPos, 1.0));

    // Transfert des coordonnées de texture
    TexCoords = aTexCoords;

    // On transforme la normale pour qu'elle suive la rotation de l'objet
    // mat3(transpose(inverse(model))) évite les bugs de normales si l'objet subit un scale
    iNormal = normalize(mat3(transpose(inverse(model))) * aNormal);

    // Position finale à l'écran
    gl_Position = projection * view * vec4(FragPos, 1.0);
}